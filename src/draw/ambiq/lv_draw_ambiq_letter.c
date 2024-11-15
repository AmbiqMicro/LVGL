/**
 * @file lv_draw_ambiq_letter.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw_label_private.h"
#include "lv_draw_ambiq.h"
#if LV_USE_DRAW_AMBIQ

#include "../../display/lv_display.h"
#include "../../misc/lv_math.h"
#include "../../misc/lv_assert.h"
#include "../../misc/lv_area.h"
#include "../../misc/lv_style.h"
#include "../../font/lv_font.h"
#include "../../core/lv_refr_private.h"
#include "../../stdlib/lv_string.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void /* LV_ATTRIBUTE_FAST_MEM */ draw_letter_cb(lv_draw_unit_t * draw_unit, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                                       lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *  GLOBAL VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_ambiq_label(lv_draw_unit_t * draw_unit, const lv_draw_label_dsc_t * dsc, const lv_area_t * coords)
{
    if(dsc->opa <= LV_OPA_MIN) return;

    LV_PROFILER_BEGIN;
    lv_draw_label_iterate_characters(draw_unit, dsc, coords, draw_letter_cb);
    LV_PROFILER_END;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void LV_ATTRIBUTE_FAST_MEM draw_letter_cb(lv_draw_unit_t * draw_unit, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                                 lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area)
{

    //lv_draw_ambiq_unit_t * draw_ambiq_unit = (lv_draw_ambiq_unit_t *)draw_unit;
    lv_layer_t * layer = draw_unit->target_layer;

    if (fill_draw_dsc == NULL && glyph_draw_dsc == NULL) {
        return;
    }
    if  (fill_draw_dsc && glyph_draw_dsc) {
        LV_LOG_WARN("Both fill and glyph draw descriptors are not NULL, use the glyph draw descriptor");
    }
    

    uint32_t color;
    if (glyph_draw_dsc) {
        color = lv_ambiq_color_convert(glyph_draw_dsc->color, glyph_draw_dsc->opa);
    }
    else {
        color = lv_ambiq_color_convert(fill_draw_dsc->color, fill_draw_dsc->opa);
    }


    lv_area_t raster_coords;

    if(glyph_draw_dsc) {
        switch(glyph_draw_dsc->format) {
            case LV_FONT_GLYPH_FORMAT_NONE: {
#if LV_USE_FONT_PLACEHOLDER
                    nema_set_blend(NEMA_BL_SIMPLE, NEMA_TEX0, NEMA_NOTEX, NEMA_NOTEX);

                    lv_area_copy(&raster_coords, glyph_draw_dsc->bg_coords);
                    lv_area_move(&raster_coords, -layer->buf_area.x1, -layer->buf_area.y1);

                    nema_draw_rect(raster_coords.x1, raster_coords.y1, 
                                   lv_area_get_width(&raster_coords), lv_area_get_height(&raster_coords),
                                   color);
#endif
                }
                break;
            case LV_FONT_GLYPH_FORMAT_A1:
            case LV_FONT_GLYPH_FORMAT_A2:
            case LV_FONT_GLYPH_FORMAT_A4:
            case LV_FONT_GLYPH_FORMAT_A8: {
                    // lv_area_t mask_area = *glyph_draw_dsc->letter_coords;
                    // mask_area.x2 = mask_area.x1 + lv_draw_buf_width_to_stride(lv_area_get_width(&mask_area), LV_COLOR_FORMAT_A8) - 1;
                    // lv_draw_ambiq_blend_dsc_t blend_dsc;
                    // lv_memzero(&blend_dsc, sizeof(blend_dsc));
                    // blend_dsc.color = glyph_draw_dsc->color;
                    // blend_dsc.opa = glyph_draw_dsc->opa;
                    // lv_draw_buf_t * draw_buf = glyph_draw_dsc->glyph_data;
                    // blend_dsc.mask_buf = draw_buf->data;
                    // blend_dsc.mask_area = &mask_area;
                    // blend_dsc.mask_stride = draw_buf->header.stride;
                    // blend_dsc.blend_area = glyph_draw_dsc->letter_coords;
                    // blend_dsc.mask_res = LV_DRAW_SW_MASK_RES_CHANGED;

                    // lv_draw_sw_blend(draw_unit, &blend_dsc);

                    lv_area_copy(&raster_coords, glyph_draw_dsc->letter_coords);
                    lv_area_move(&raster_coords, -layer->buf_area.x1, -layer->buf_area.y1);

                    lv_draw_buf_t * draw_buf = glyph_draw_dsc->glyph_data;

                    if ( (color & 0xFF000000U) == 0xFF000000U) {
                        nema_set_blend_blit(NEMA_BL_SIMPLE);
                    } else {
                        nema_set_blend_blit(NEMA_BL_SIMPLE|NEMA_BLOP_MODULATE_A);
                        nema_set_const_color(color); 
                    }
                    nema_set_tex_color(color);
                    nema_bind_src_tex((uintptr_t)draw_buf->data, draw_buf->header.w, draw_buf->header.h, NEMA_A8, draw_buf->header.stride, NEMA_FILTER_PS);

                    nema_matrix3x3_t m;
                    nema_mat3x3_load_identity(m);
                    nema_mat3x3_translate(m, -raster_coords.x1, -raster_coords.y1);
                    nema_set_matrix(m);
                    //nema_set_matrix_translate(raster_coords.x1, raster_coords.y1);
                    nema_raster_rect(raster_coords.x1, raster_coords.y1, raster_coords.x2 - raster_coords.x1 + 1, raster_coords.y2 - raster_coords.y1 + 1);
                }
                break;
            case LV_FONT_GLYPH_FORMAT_IMAGE: {
#if LV_USE_IMGFONT
                    lv_draw_image_dsc_t img_dsc;
                    lv_draw_image_dsc_init(&img_dsc);
                    img_dsc.rotation = 0;
                    img_dsc.scale_x = LV_SCALE_NONE;
                    img_dsc.scale_y = LV_SCALE_NONE;
                    img_dsc.opa = glyph_draw_dsc->opa;
                    img_dsc.src = glyph_draw_dsc->glyph_data;
                    lv_draw_ambiq_image(draw_unit, &img_dsc, glyph_draw_dsc->letter_coords);
#endif
                }
                break;
            default:
                break;
        }

    }

    if(fill_draw_dsc && fill_area) {
        nema_set_blend(NEMA_BL_SIMPLE, NEMA_TEX0, NEMA_NOTEX, NEMA_NOTEX);

        lv_area_copy(&raster_coords, fill_area);
        lv_area_move(&raster_coords, -layer->buf_area.x1, -layer->buf_area.y1);

        nema_fill_rect(raster_coords.x1, raster_coords.y1, 
                        lv_area_get_width(&raster_coords), lv_area_get_height(&raster_coords),
                        color);
    }


    nema_cmdlist_t * current_cl = nema_cl_get_bound();
    nema_cl_submit(current_cl);
    nema_cl_wait(current_cl);
    nema_cl_rewind(current_cl);
}

#endif /*LV_USE_DRAW_AMBIQ*/
