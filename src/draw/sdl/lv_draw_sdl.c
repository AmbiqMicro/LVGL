/**
 * @file lv_draw_sdl.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#if LV_USE_DRAW_SDL
#include LV_SDL_INCLUDE_PATH
#include <SDL2/SDL_image.h>

#include "lv_draw_sdl.h"
#include "../../core/lv_refr.h"
#include "../../display/lv_display_private.h"
#include "../../stdlib/lv_string.h"
#include "../../drivers/sdl/lv_sdl_window.h"
#include "../../misc/cache/lv_cache_entry_private.h"

/*********************
 *      DEFINES
 *********************/
#define DRAW_UNIT_ID_SDL     100

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_draw_dsc_base_t * draw_dsc;
    int32_t w;
    int32_t h;
    SDL_Texture * texture;
} cache_data_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void execute_drawing(lv_draw_sdl_unit_t * u);

static int32_t dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer);

static int32_t evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task);
static bool draw_to_texture(lv_draw_sdl_unit_t * u, cache_data_t * cache_data);

/**********************
 *  GLOBAL PROTOTYPES
 **********************/
static uint8_t sdl_render_buf[2048 * 1024 * 4];

/**********************
 *  STATIC VARIABLES
 **********************/
static SDL_Texture * layer_get_texture(lv_layer_t * layer);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
static bool sdl_texture_cache_create_cb(cache_data_t * cached_data, void * user_data)
{
    return draw_to_texture((lv_draw_sdl_unit_t *)user_data, cached_data);
}

static void sdl_texture_cache_free_cb(cache_data_t * cached_data, void * user_data)
{
    LV_UNUSED(user_data);

    lv_free(cached_data->draw_dsc);
    SDL_DestroyTexture(cached_data->texture);
    cached_data->draw_dsc = NULL;
    cached_data->texture = NULL;
}

static lv_cache_compare_res_t sdl_texture_cache_compare_cb(const cache_data_t * lhs, const cache_data_t * rhs)
{
    if(lhs == rhs) return 0;

    if(lhs->w != rhs->w) {
        return lhs->w > rhs->w ? 1 : -1;
    }
    if(lhs->h != rhs->h) {
        return lhs->h > rhs->h ? 1 : -1;
    }

    uint32_t lhs_dsc_size = lhs->draw_dsc->dsc_size;
    uint32_t rhs_dsc_size = rhs->draw_dsc->dsc_size;

    if(lhs_dsc_size != rhs_dsc_size) {
        return lhs_dsc_size > rhs_dsc_size ? 1 : -1;
    }

    int cmp_res = memcmp(lhs->draw_dsc, rhs->draw_dsc, lhs->draw_dsc->dsc_size);

    if(cmp_res != 0) {
        return cmp_res > 0 ? 1 : -1;
    }

    return 0;
}

void lv_draw_sdl_init(void)
{
    lv_draw_sdl_unit_t * draw_sdl_unit = lv_draw_create_unit(sizeof(lv_draw_sdl_unit_t));
    draw_sdl_unit->base_unit.dispatch_cb = dispatch;
    draw_sdl_unit->base_unit.evaluate_cb = evaluate;
    draw_sdl_unit->texture_cache = lv_cache_create(&lv_cache_class_lru_rb_count,
    sizeof(cache_data_t), 128, (lv_cache_ops_t) {
        .compare_cb = (lv_cache_compare_cb_t)sdl_texture_cache_compare_cb,
        .create_cb = (lv_cache_create_cb_t)sdl_texture_cache_create_cb,
        .free_cb = (lv_cache_free_cb_t)sdl_texture_cache_free_cb,
    });
    lv_cache_set_name(draw_sdl_unit->texture_cache, "SDL_TEXTURE");
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int32_t dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer)
{
    lv_draw_sdl_unit_t * draw_sdl_unit = (lv_draw_sdl_unit_t *) draw_unit;

    /*Return immediately if it's busy with a draw task*/
    if(draw_sdl_unit->task_act) return 0;

    lv_draw_task_t * t = NULL;
    t = lv_draw_get_next_available_task(layer, NULL, DRAW_UNIT_ID_SDL);
    if(t == NULL) return -1;

    lv_display_t * disp = _lv_refr_get_disp_refreshing();
    SDL_Texture * texture = layer_get_texture(layer);
    if(layer != disp->layer_head && texture == NULL) {
        void * buf = lv_draw_layer_alloc_buf(layer);
        if(buf == NULL) return -1;

        SDL_Renderer * renderer = lv_sdl_window_get_renderer(disp);
        int32_t w = lv_area_get_width(&layer->buf_area);
        int32_t h = lv_area_get_height(&layer->buf_area);
        layer->user_data = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_TARGET, w, h);
    }

    t->state = LV_DRAW_TASK_STATE_IN_PROGRESS;
    draw_sdl_unit->base_unit.target_layer = layer;
    draw_sdl_unit->base_unit.clip_area = &t->clip_area;
    draw_sdl_unit->task_act = t;

    execute_drawing(draw_sdl_unit);

    draw_sdl_unit->task_act->state = LV_DRAW_TASK_STATE_READY;
    draw_sdl_unit->task_act = NULL;

    /*The draw unit is free now. Request a new dispatching as it can get a new task*/
    lv_draw_dispatch_request();
    return 1;
}

static int32_t evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task)
{
    LV_UNUSED(draw_unit);

    if(((lv_draw_dsc_base_t *)task->draw_dsc)->user_data == NULL) {
        task->preference_score = 0;
        task->preferred_draw_unit_id = DRAW_UNIT_ID_SDL;
    }
    return 0;
}

static bool draw_to_texture(lv_draw_sdl_unit_t * u, cache_data_t * cache_data)
{
    lv_draw_task_t * task = u->task_act;

    lv_layer_t dest_layer;
    lv_memzero(&dest_layer, sizeof(dest_layer));

    int32_t texture_w = lv_area_get_width(&task->_real_area);
    int32_t texture_h = lv_area_get_height(&task->_real_area);

    lv_draw_buf_t draw_buf;
    dest_layer.draw_buf = &draw_buf;
    lv_draw_buf_init(dest_layer.draw_buf, texture_w, texture_h,
                     LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO, sdl_render_buf, sizeof(sdl_render_buf));
    dest_layer.color_format = LV_COLOR_FORMAT_ARGB8888;
    dest_layer.buf_area = task->_real_area;
    dest_layer._clip_area = task->_real_area;
    lv_memzero(sdl_render_buf, lv_area_get_size(&dest_layer.buf_area) * 4);

    lv_display_t * disp = _lv_refr_get_disp_refreshing();

    lv_obj_t * obj = ((lv_draw_dsc_base_t *)task->draw_dsc)->obj;
    bool original_send_draw_task_event = false;
    if(obj) {
        original_send_draw_task_event = lv_obj_has_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    }

    SDL_Texture * texture = NULL;
    switch(task->type) {
        case LV_DRAW_TASK_TYPE_FILL: {
                lv_draw_fill_dsc_t * fill_dsc = task->draw_dsc;
                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                rect_dsc.bg_color = fill_dsc->color;
                rect_dsc.bg_grad = fill_dsc->grad;
                rect_dsc.radius = fill_dsc->radius;
                rect_dsc.bg_opa = fill_dsc->opa;

                lv_draw_rect(&dest_layer, &rect_dsc, &task->area);
            }
            break;
        case LV_DRAW_TASK_TYPE_BORDER: {
                lv_draw_border_dsc_t * border_dsc = task->draw_dsc;
                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                rect_dsc.bg_opa = LV_OPA_TRANSP;
                rect_dsc.radius = border_dsc->radius;
                rect_dsc.border_color = border_dsc->color;
                rect_dsc.border_opa = border_dsc->opa;
                rect_dsc.border_side = border_dsc->side;
                rect_dsc.border_width = border_dsc->width;
                lv_draw_rect(&dest_layer, &rect_dsc, &task->area);
                break;
            }
        case LV_DRAW_TASK_TYPE_BOX_SHADOW: {
                lv_draw_box_shadow_dsc_t * box_shadow_dsc = task->draw_dsc;
                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                rect_dsc.bg_opa = LV_OPA_0;
                rect_dsc.radius = box_shadow_dsc->radius;
                rect_dsc.bg_color = box_shadow_dsc->color;
                rect_dsc.shadow_opa = LV_OPA_20;//box_shadow_dsc->opa;
                rect_dsc.shadow_width = box_shadow_dsc->width;
                rect_dsc.shadow_spread = box_shadow_dsc->spread;
                rect_dsc.shadow_offset_x = box_shadow_dsc->ofs_x;
                rect_dsc.shadow_offset_y = box_shadow_dsc->ofs_y;
                lv_draw_rect(&dest_layer, &rect_dsc, &task->area);
                break;
            }
        case LV_DRAW_TASK_TYPE_LABEL: {
                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                lv_memcpy(&label_dsc, task->draw_dsc, sizeof(label_dsc));
                label_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                lv_draw_label(&dest_layer, &label_dsc, &task->area);
            }
            break;
        case LV_DRAW_TASK_TYPE_ARC: {
                lv_draw_arc_dsc_t arc_dsc;
                lv_draw_arc_dsc_init(&arc_dsc);
                lv_memcpy(&arc_dsc, task->draw_dsc, sizeof(arc_dsc));
                arc_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                lv_draw_arc(&dest_layer, &arc_dsc);
            }
            break;
        case LV_DRAW_TASK_TYPE_LINE: {
                lv_draw_line_dsc_t line_dsc;
                lv_draw_line_dsc_init(&line_dsc);
                lv_memcpy(&line_dsc, task->draw_dsc, sizeof(line_dsc));
                line_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                lv_draw_line(&dest_layer, &line_dsc);
            }
            break;
        case LV_DRAW_TASK_TYPE_TRIANGLE: {
                lv_draw_triangle_dsc_t triangle_dsc;
                lv_draw_triangle_dsc_init(&triangle_dsc);
                lv_memcpy(&triangle_dsc, task->draw_dsc, sizeof(triangle_dsc));
                triangle_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                lv_draw_triangle(&dest_layer, &triangle_dsc);
            }
            break;
        case LV_DRAW_TASK_TYPE_IMAGE: {
                lv_draw_image_dsc_t * image_dsc = task->draw_dsc;
                lv_image_src_t type = lv_image_src_get_type(image_dsc->src);
                SDL_Surface * surface = NULL;
                if(type == LV_IMAGE_SRC_FILE) {
                    const char * path = image_dsc->src;
                    surface = IMG_Load(&path[2]);
                    if(surface == NULL) {
                        LV_LOG_ERROR("could not load image from file: %s", IMG_GetError());
                        return false;
                    }
                }
                else if(type == LV_IMAGE_SRC_VARIABLE) {
                    lv_image_dsc_t * lvd = image_dsc->src;
                    surface = SDL_CreateRGBSurfaceFrom(lvd->data,
                                                       lvd->header.w, lvd->header.h,
                                                       LV_COLOR_FORMAT_GET_BPP(lvd->header.cf),
                                                       lvd->header.stride,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                                       0x00FF0000,
                                                       0x0000FF00,
                                                       0x000000FF,
                                                       0xFF000000
#else
                                                       0x0000FF00,
                                                       0x00FF0000,
                                                       0xFF000000,
                                                       0x000000FF
#endif
                                                      );
                    if(surface == NULL) {
                        LV_LOG_ERROR("could not load image from variable");
                        return false;
                    }
                }
                else {
                    LV_LOG_WARN("image source type unknown");
                    return false;
                }

                SDL_Renderer * renderer = lv_sdl_window_get_renderer(disp);
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                break;
            }
        default:
            return false;
    }


    while(dest_layer.draw_task_head) {
        lv_draw_dispatch_layer(disp, &dest_layer);
        if(dest_layer.draw_task_head) {
            lv_draw_dispatch_wait_for_request();
        }
    }

    if(texture == NULL) {
        texture = SDL_CreateTexture(lv_sdl_window_get_renderer(disp), SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STATIC, texture_w, texture_h);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_UpdateTexture(texture, NULL, sdl_render_buf, texture_w * 4);
    }
    else {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    lv_draw_dsc_base_t * base_dsc = task->draw_dsc;

    cache_data->draw_dsc = lv_malloc(base_dsc->dsc_size);
    lv_memcpy((void *)cache_data->draw_dsc, base_dsc, base_dsc->dsc_size);
    cache_data->w = texture_w;
    cache_data->h = texture_h;
    cache_data->texture = texture;

    if(obj) {
        lv_obj_update_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS, original_send_draw_task_event);
    }

    return true;
}

static void blend_texture_layer(lv_draw_sdl_unit_t * u)
{
    lv_display_t * disp = _lv_refr_get_disp_refreshing();
    SDL_Renderer * renderer = lv_sdl_window_get_renderer(disp);

    SDL_Rect clip_rect;
    clip_rect.x = u->base_unit.clip_area->x1;
    clip_rect.y = u->base_unit.clip_area->y1;
    clip_rect.w = lv_area_get_width(u->base_unit.clip_area);
    clip_rect.h = lv_area_get_height(u->base_unit.clip_area);

    lv_draw_task_t * t = u->task_act;
    SDL_Rect rect;
    rect.x = t->area.x1;
    rect.y = t->area.y1;
    rect.w = lv_area_get_width(&t->area);
    rect.h = lv_area_get_height(&t->area);

    lv_draw_image_dsc_t * draw_dsc = t->draw_dsc;
    lv_layer_t * src_layer = (lv_layer_t *)draw_dsc->src;
    SDL_Texture * src_texture = layer_get_texture(src_layer);

    SDL_SetTextureAlphaMod(src_texture, draw_dsc->opa);
    SDL_SetTextureBlendMode(src_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, layer_get_texture(u->base_unit.target_layer));
    SDL_RenderSetClipRect(renderer, &clip_rect);
    SDL_RenderCopy(renderer, src_texture, NULL, &rect);
    SDL_DestroyTexture(src_texture);
    SDL_RenderSetClipRect(renderer, NULL);
}

static void draw_from_cached_texture(lv_draw_sdl_unit_t * u)
{
    lv_draw_task_t * t = u->task_act;

    cache_data_t data_to_find;
    data_to_find.draw_dsc = (lv_draw_dsc_base_t *)t->draw_dsc;

    data_to_find.w = lv_area_get_width(&t->area);
    data_to_find.h = lv_area_get_height(&t->area);
    data_to_find.texture = NULL;

    /*user_data stores the renderer to differentiate it from SW rendered tasks.
     *However the cached texture is independent from the renderer so use NULL user_data*/
    void * user_data_saved = data_to_find.draw_dsc->user_data;
    data_to_find.draw_dsc->user_data = NULL;

    lv_cache_entry_t * entry_cached = lv_cache_acquire_or_create(u->texture_cache, &data_to_find, u);
    if(!entry_cached) {
        return;
    }

    data_to_find.draw_dsc->user_data = user_data_saved;

    cache_data_t * data_cached = lv_cache_entry_get_data(entry_cached);
    SDL_Texture * texture = data_cached->texture;
    lv_display_t * disp = _lv_refr_get_disp_refreshing();
    SDL_Renderer * renderer = lv_sdl_window_get_renderer(disp);

    lv_layer_t * dest_layer = u->base_unit.target_layer;
    SDL_Rect clip_rect;
    clip_rect.x = u->base_unit.clip_area->x1 - dest_layer->buf_area.x1;
    clip_rect.y = u->base_unit.clip_area->y1 - dest_layer->buf_area.y1;
    clip_rect.w = lv_area_get_width(u->base_unit.clip_area);
    clip_rect.h = lv_area_get_height(u->base_unit.clip_area);

    SDL_Rect rect;

    SDL_SetRenderTarget(renderer, layer_get_texture(dest_layer));

    lv_draw_image_dsc_t * draw_dsc = lv_draw_task_get_image_dsc(t);
    if(draw_dsc) {
        lv_area_t image_area;
        image_area.x1 = 0;
        image_area.y1 = 0;
        image_area.x2 = draw_dsc->header.w - 1;
        image_area.y2 = draw_dsc->header.h - 1;

        lv_area_move(&image_area, t->area.x1 - dest_layer->buf_area.x1, t->area.y1 - dest_layer->buf_area.y1);
        rect.x = image_area.x1;
        rect.y = image_area.y1;
        rect.w = lv_area_get_width(&image_area);
        rect.h = lv_area_get_height(&image_area);

        SDL_RenderSetClipRect(renderer, &clip_rect);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }
    else {
        rect.x = t->_real_area.x1 - dest_layer->buf_area.x1;
        rect.y = t->_real_area.y1 - dest_layer->buf_area.y1;
        rect.w = data_cached->w;
        rect.h = data_cached->h;

        SDL_RenderSetClipRect(renderer, &clip_rect);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }

    SDL_RenderSetClipRect(renderer, NULL);

    /*Do not cache label's with local text as the text will be freed*/
    if(t->type == LV_DRAW_TASK_TYPE_LABEL) {
        lv_draw_label_dsc_t * label_dsc = t->draw_dsc;
        if(label_dsc->text_local) {
            //          lv_cache_entry_set_invalid(entry_cached, true);
            printf("a\n");
        }
    }
    printf("b\n");
    lv_cache_release(u->texture_cache, entry_cached, u);
    printf("c\n");
}

static void execute_drawing(lv_draw_sdl_unit_t * u)
{
    lv_draw_task_t * t = u->task_act;

    if(t->type == LV_DRAW_TASK_TYPE_LAYER) {
        blend_texture_layer(u);
    }
    else {
        draw_from_cached_texture(u);
    }
}

static SDL_Texture * layer_get_texture(lv_layer_t * layer)
{
    return layer->user_data;
}

#endif /*LV_USE_DRAW_SDL*/
