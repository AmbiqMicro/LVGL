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
#include <math.h>

#include "nema_font_loader.h"
#include "nema_vg.h"

/*********************
 *      DEFINES
 *********************/
typedef struct {
    uint32_t glyph_data_length;
    uint32_t glyph_data_offset;
    NEMA_VG_PATH_HANDLE handle;
} lv_ambiq_glyph_data_internal_t;


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool ttf_get_glyph_dsc_cb(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter,
                                 uint32_t unicode_letter_next);
static const void * ttf_get_glyph_bitmap_cb(lv_font_glyph_dsc_t * g_dsc, lv_draw_buf_t * draw_buf);
static void ttf_release_glyph_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc);
static lv_font_t * lv_ambiq_ttf_create_internal(ambiq_vg_font_t * nema_font_handle, int32_t font_size);

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
        LV_LOG_WARN("Invalid font size specified: %d", font_size);
        return;
    }

    ambiq_vg_font_t * nema_font = (ambiq_vg_font_t *)font->dsc;
    ambiq_vg_font_metrics_t unscaled_metrics;
    if(nema_font_get_metrics(nema_font, &unscaled_metrics) != LV_RES_OK) {
        return;
    }

    float scale = 1.0f;
    if(unscaled_metrics.line_height > 0) {
        scale = (float)font_size / (float)unscaled_metrics.line_height;
    }

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
        nema_font_unload((ambiq_vg_font_t *)font->dsc);
    }
    lv_free(font);
}

float lv_ambiq_ttf_get_scale(const lv_font_t * font)
{
    if(!font || !font->dsc) {
        LV_LOG_WARN("Cannot get scale from an invalid font handle.");
        return 1.0f;
    }

    const ambiq_vg_font_t * nema_font = (const ambiq_vg_font_t *)font->dsc;
    ambiq_vg_font_metrics_t unscaled_metrics;
    if(nema_font_get_metrics(nema_font, &unscaled_metrics) != LV_RES_OK) {
        LV_LOG_WARN("Failed to get unscaled metrics; cannot calculate scale.");
        return 1.0f;
    }

    if(unscaled_metrics.line_height <= 0) {
        LV_LOG_WARN("Font has an invalid original line_height (<= 0); cannot calculate scale.");
        return 1.0f;
    }

    return (float)font->line_height / (float)unscaled_metrics.line_height;
}

// void lv_ambiq_ttf_fill_path_from_glyph(NEMA_VG_PATH_HANDLE path, const void * glyph_data)
// {
//     if(!path || !glyph_data) {
//         return;
//     }
//     const nema_glyph_t * nema_ttf_glyph = (const nema_glyph_t *)glyph_data;
//     if(nema_ttf_glyph->data_length_bytes == 0 || nema_ttf_glyph->segment_length_bytes == 0) {
//         return;
//     }
//     const float * coords_ptr = NEMA_FONT_GET_COORDS_PTR(nema_ttf_glyph);
//     const uint8_t * segments_ptr = NEMA_FONT_GET_SEGMENTS_PTR(nema_ttf_glyph);
//     const float bbox[4] = {
//         (float)nema_ttf_glyph->bbox_xmin, (float)nema_ttf_glyph->bbox_ymin,
//         (float)nema_ttf_glyph->bbox_xmax, (float)nema_ttf_glyph->bbox_ymax
//     };
//     size_t num_coords = nema_ttf_glyph->data_length_bytes / sizeof(float);
//     size_t num_segments = nema_ttf_glyph->segment_length_bytes;
//     nema_vg_path_set_shape_and_bbox(path, num_segments, segments_ptr, num_coords, coords_ptr, bbox);
// }

bool lv_ambiq_ttf_identify(const lv_font_t * font)
{
    if(!font || !font->dsc) {
        return false;
    }
    const ambiq_vg_font_t * nema_font = (const ambiq_vg_font_t *)font->dsc;
    return nema_font_is_valid(nema_font);
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

    new_font->dsc = nema_font_handle;
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
    if(!font || !font->dsc || !dsc_out) {
        return false;
    }

    LV_UNUSED(unicode_letter_next);

    const ambiq_vg_font_t * nema_font = (const ambiq_vg_font_t *)font->dsc;
    ambiq_vg_glyph_metrics_t glyph_metic;
    lv_result_t res = nema_font_get_glyph_metrics(nema_font, unicode_letter, &glyph_metic);
    if(res != LV_RES_OK) {
        return false;
    }


    float scale = lv_ambiq_ttf_get_scale(font);
    dsc_out->adv_w = (uint16_t)roundf(glyph_metic.xAdvance * scale);
    dsc_out->box_w = (uint16_t)roundf((glyph_metic.bbox_xmax - glyph_metic.bbox_xmin) * scale);
    dsc_out->box_h = (uint16_t)roundf((glyph_metic.bbox_ymax - glyph_metic.bbox_ymin) * scale);
    dsc_out->ofs_x = (int16_t)roundf(glyph_metic.bbox_xmin * scale);
    dsc_out->ofs_y = (int16_t)roundf(-glyph_metic.bbox_ymax * scale);
    dsc_out->format = LV_FONT_GLYPH_FORMAT_VECTOR;
    dsc_out->is_placeholder = 0;
    dsc_out->outline_stroke_width = 0;
    dsc_out->gid.index = glyph_metic.glyph_index;
    return true;
}

static const void * ttf_get_glyph_bitmap_cb(lv_font_glyph_dsc_t * g_dsc, lv_draw_buf_t * draw_buf)
{
    LV_UNUSED(draw_buf);
    if(!g_dsc) {
        return NULL;
    }

    const lv_font_t * font = g_dsc->resolved_font;
    if(font->dsc == NULL) {
        return NULL;
    }

    const ambiq_vg_font_t * nema_font = (const ambiq_vg_font_t *)font->dsc;

    NEMA_VG_PATH_HANDLE path = nema_font_get_glyph_shape(nema_font, g_dsc->gid.index);

    g_dsc->entry = (lv_cache_entry_t *)path;
    return (void *)path;
}

static void ttf_release_glyph_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc)
{
    LV_UNUSED(font);
    if(!g_dsc || g_dsc->entry == NULL) {
        return;
    }

    nema_font_free_glyph_shape((NEMA_VG_PATH_HANDLE)g_dsc->entry);
    g_dsc->entry = NULL;
}

#endif // LV_USE_AMBIQ_TTF