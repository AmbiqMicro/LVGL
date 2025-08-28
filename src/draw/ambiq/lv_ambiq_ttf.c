/**
 * @file lv_ambiq_ttf.c
 * @brief An adapter layer to integrate the NEMA custom vector font loader with LVGL's font engine.
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../lvgl.h"

#if LV_USE_AMBIQ_TTF
#include "../../core/lv_global.h"
#include "../../misc/cache/lv_cache.h"
#include <math.h>

#include "nema_font_loader.h"
#include "nema_vg.h"

/*********************
 *      DEFINES
 *********************/
#define CACHE_NODE_SIZE sizeof(lv_ambiq_glyph_cache_t)

#ifndef LV_AMBIQ_TTF_CACHE_SIZE
    #define LV_AMBIQ_TTF_CACHE_SIZE 100
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    ambiq_vg_font_t * nema_font;
    lv_cache_t   *   glyph_cache;
    float            scale;
} lv_ambiq_ttf_dsc_t;

typedef struct {
    uint32_t unicode;
    ambiq_vg_glyph_metrics_t glyph_info;
    NEMA_VG_PATH_HANDLE path_handle;
} lv_ambiq_glyph_cache_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool ttf_get_glyph_dsc_cb(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter,
                                 uint32_t unicode_letter_next);
static const void * ttf_get_glyph_bitmap_cb(lv_font_glyph_dsc_t * g_dsc, lv_draw_buf_t * draw_buf);
static void ttf_release_glyph_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc);
static lv_font_t * lv_ambiq_ttf_create_internal(ambiq_vg_font_t * nema_font_handle, int32_t font_size);

static bool ambiq_glyph_cache_create_cb(lv_ambiq_glyph_cache_t * node, void * user_data);
static void ambiq_glyph_cache_free_cb(lv_ambiq_glyph_cache_t * node, void * user_data);
static lv_cache_compare_res_t ambiq_glyph_cache_compare_cb(const lv_ambiq_glyph_cache_t * lhs,
                                                           const lv_ambiq_glyph_cache_t * rhs);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_font_t * lv_ambiq_ttf_create_file(const char * path, int32_t font_size)
{
    if(!path) {
        LV_LOG_ERROR("path is NULL.");
        return NULL;
    }

    ambiq_vg_font_t * nema_font_handle = NULL;
    lv_result_t res = nema_font_load(path, &nema_font_handle);

    if(res != LV_RESULT_OK || nema_font_handle == NULL) {
        LV_LOG_ERROR("Failed to load nema_font data from path: %s", path);
        return NULL;
    }

    return lv_ambiq_ttf_create_internal(nema_font_handle, font_size);
}

lv_font_t * lv_ambiq_ttf_create_data(const void * data, size_t data_size, int32_t font_size)
{
    if(!data || data_size == 0) {
        LV_LOG_ERROR("data is NULL or data_size is 0.");
        return NULL;
    }

    ambiq_vg_font_t * nema_font_handle = NULL;
    lv_result_t res = nema_font_load_from_buffer(data, data_size, &nema_font_handle);

    if(res != LV_RESULT_OK || nema_font_handle == NULL) {
        LV_LOG_ERROR("Failed to load nema_font data from buffer.");
        return NULL;
    }

    return lv_ambiq_ttf_create_internal(nema_font_handle, font_size);
}

void lv_ambiq_ttf_set_size(lv_font_t * font, int32_t font_size)
{
    if(!font || !font->dsc) {
        return;
    }
    if(font_size <= 0) {
        LV_LOG_WARN("Invalid font size specified: %ld", font_size);
        return;
    }

    lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)font->dsc;
    ambiq_vg_font_metrics_t unscaled_metrics;
    if(nema_font_get_metrics(dsc->nema_font, &unscaled_metrics) != LV_RES_OK) {
        return;
    }

    float scale = 1.0f;
    if(unscaled_metrics.line_height > 0) {
        scale = (float)font_size / (float)unscaled_metrics.line_height;
    }
    dsc->scale = scale;

    font->line_height = font_size;
    font->base_line = (int32_t)roundf(unscaled_metrics.base_line * scale);
    font->underline_position = (int8_t)roundf(unscaled_metrics.underline_position * scale);
    float scaled_thickness = unscaled_metrics.underline_thickness * scale;
    font->underline_thickness = (int8_t)fmaxf(1.0f, roundf(scaled_thickness));
}

void lv_ambiq_ttf_destroy(lv_font_t * font)
{
    if(!font) return;
    if(font->dsc) {
        lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)font->dsc;
        nema_font_unload(dsc->nema_font);
        if(dsc->glyph_cache) {
            lv_cache_destroy(dsc->glyph_cache, NULL);
        }
        lv_free(dsc);
    }
    lv_free(font);
}

float lv_ambiq_ttf_get_scale(const lv_font_t * font)
{
    if(!font || !font->dsc) {
        LV_LOG_WARN("Cannot get scale from an invalid font handle.");
        return 1.0f;
    }

    lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)font->dsc;
    return dsc->scale;
}

bool lv_ambiq_ttf_identify(const lv_font_t * font)
{
    if(!font || !font->dsc) {
        return false;
    }
    const lv_ambiq_ttf_dsc_t * dsc = (const lv_ambiq_ttf_dsc_t *)font->dsc;
    return nema_font_is_valid(dsc->nema_font);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static lv_font_t * lv_ambiq_ttf_create_internal(ambiq_vg_font_t * nema_font_handle, int32_t font_size)
{
    if(font_size <= 0) {
        LV_LOG_ERROR("Invalid font_size (<= 0).");
        nema_font_unload(nema_font_handle);
        return NULL;
    }

    lv_font_t * new_font = (lv_font_t *)lv_malloc_zeroed(sizeof(lv_font_t));
    if(new_font == NULL) {
        LV_LOG_ERROR("Failed to allocate memory for lv_font_t.");
        nema_font_unload(nema_font_handle);
        return NULL;
    }

    lv_ambiq_ttf_dsc_t * dsc = lv_malloc_zeroed(sizeof(lv_ambiq_ttf_dsc_t));
    if(dsc == NULL) {
        LV_LOG_ERROR("Failed to allocate memory for lv_ambiq_ttf_dsc_t.");
        nema_font_unload(nema_font_handle);
        lv_free(new_font);
        return NULL;
    }

    dsc->nema_font = nema_font_handle;
    dsc->glyph_cache = lv_cache_create(&lv_cache_class_lru_rb_count, CACHE_NODE_SIZE, LV_AMBIQ_TTF_CACHE_SIZE,
    (lv_cache_ops_t) {
        .compare_cb = (lv_cache_compare_cb_t)ambiq_glyph_cache_compare_cb,
        .create_cb = (lv_cache_create_cb_t)ambiq_glyph_cache_create_cb,
        .free_cb = (lv_cache_free_cb_t)ambiq_glyph_cache_free_cb,
    });
    lv_cache_set_name(dsc->glyph_cache, "AMBIQ_TTF_GLYPH");

    new_font->dsc = dsc;
    new_font->get_glyph_dsc = ttf_get_glyph_dsc_cb;
    new_font->get_glyph_bitmap = ttf_get_glyph_bitmap_cb;
    new_font->release_glyph = ttf_release_glyph_cb;
    new_font->kerning = LV_FONT_KERNING_NONE;

    lv_ambiq_ttf_set_size(new_font, font_size);

    return new_font;
}

static bool ttf_get_glyph_dsc_cb(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter,
                                 uint32_t unicode_letter_next)
{
    LV_UNUSED(unicode_letter_next);
    if(!font || !font->dsc || !dsc_out) {
        return false;
    }

    lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)font->dsc;

    lv_ambiq_glyph_cache_t search_key = {.unicode = unicode_letter};
    lv_cache_entry_t * entry = lv_cache_acquire_or_create(dsc->glyph_cache, &search_key, dsc);
    if(entry == NULL) {
        return false;
    }

    lv_ambiq_glyph_cache_t * data = lv_cache_entry_get_data(entry);

    float scale = dsc->scale;
    dsc_out->adv_w = (uint16_t)roundf(data->glyph_info.xAdvance * scale);
    dsc_out->box_w = (uint16_t)roundf((data->glyph_info.bbox_xmax - data->glyph_info.bbox_xmin) * scale);
    dsc_out->box_h = (uint16_t)roundf((data->glyph_info.bbox_ymax - data->glyph_info.bbox_ymin) * scale);
    dsc_out->ofs_x = (int16_t)roundf(data->glyph_info.bbox_xmin * scale);
    dsc_out->ofs_y = (int16_t)roundf(-data->glyph_info.bbox_ymax * scale);
    dsc_out->format = LV_FONT_GLYPH_FORMAT_VECTOR;
    dsc_out->is_placeholder = 0;
    dsc_out->outline_stroke_width = 0;
    dsc_out->gid.index = unicode_letter;

    dsc_out->entry = NULL;

    lv_cache_release(dsc->glyph_cache, entry, NULL);

    return true;
}

static const void * ttf_get_glyph_bitmap_cb(lv_font_glyph_dsc_t * g_dsc, lv_draw_buf_t * draw_buf)
{
    LV_UNUSED(draw_buf);

    lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)g_dsc->resolved_font->dsc;

    lv_ambiq_glyph_cache_t search_key = {.unicode = g_dsc->gid.index};
    lv_cache_entry_t * entry = lv_cache_acquire(dsc->glyph_cache, &search_key, dsc);
    if(entry == NULL) {
        return NULL;
    }

    g_dsc->entry = entry;

    lv_ambiq_glyph_cache_t * data = lv_cache_entry_get_data(entry);

    if(data->path_handle == NULL) {
        const lv_font_t * font = g_dsc->resolved_font;
        lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)font->dsc;
        data->path_handle = nema_font_get_glyph_shape_from_info(dsc->nema_font, data->glyph_info.glyph_data_offset,
                                                                data->glyph_info.glyph_data_length);
    }

    return data->path_handle;
}

static void ttf_release_glyph_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc)
{
    if(g_dsc->entry == NULL) {
        return;
    }
    lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)font->dsc;
    lv_cache_release(dsc->glyph_cache, g_dsc->entry, NULL);
    g_dsc->entry = NULL;
}

static bool ambiq_glyph_cache_create_cb(lv_ambiq_glyph_cache_t * node, void * user_data)
{
    lv_ambiq_ttf_dsc_t * dsc = (lv_ambiq_ttf_dsc_t *)user_data;

    lv_result_t res = nema_font_get_glyph_info(dsc->nema_font, node->unicode, &node->glyph_info);
    if(res != LV_RESULT_OK) {
        return false;
    }

    node->path_handle = NULL;
    return true;
}

static void ambiq_glyph_cache_free_cb(lv_ambiq_glyph_cache_t * node, void * user_data)
{
    LV_UNUSED(user_data);
    if(node->path_handle != NULL) {
        nema_font_free_glyph_shape(node->path_handle);
    }
}

static lv_cache_compare_res_t ambiq_glyph_cache_compare_cb(const lv_ambiq_glyph_cache_t * lhs,
                                                           const lv_ambiq_glyph_cache_t * rhs)
{
    if(lhs->unicode != rhs->unicode) {
        return lhs->unicode > rhs->unicode ? 1 : -1;
    }
    return 0;
}

#endif // LV_USE_AMBIQ_TTF