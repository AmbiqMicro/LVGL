/**
 * @file lv_ambiq_ttf.c
 * @brief An adapter layer to integrate the NEMA custom vector font loader with LVGL's font engine.
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../lvgl.h"
#include "src/misc/lv_assert.h"
#include "src/misc/lv_log.h"

#if LV_USE_AMBIQ_TTF
#include "../../core/lv_global.h"
#include "../../misc/cache/lv_cache.h"
#include <string.h>
#include <math.h>

#include "lv_ambiq_ttf.h"
#include "nema_vg.h"
#include "gpu_patch.h"
#include "lv_draw_ambiq_private.h"

#include "am_debug_pin.h"
#include "am_mem.h"

/*********************
 *      DEFINES
 *********************/

// --- Defines ---
#define FONT_FILE_MAGIC 0x4E464F4E
#define FONT_FILE_VERSION 6

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_ambiq_ttf_handle_t * nema_font;
    lv_cache_t   *   bitmap_cache;
    float            scale;
    int32_t          font_size;
} adapter_ctx_t;

typedef struct {
    uint32_t glyph_index;
    lv_draw_buf_t * bitmap;
    int cl_id;
} bitmap_cache_node_t;

typedef struct {
    adapter_ctx_t * dsc;
    uint32_t glyph_index;
    int32_t box_w;
    int32_t box_h;
} bitmap_create_ctx_t;

// --- I/O Stream Abstraction ---

/**
 * @brief Defines the source type for the font data stream.
 */
typedef enum {
    FONT_STREAM_TYPE_UNKNOWN,
    FONT_STREAM_TYPE_FILE,
    FONT_STREAM_TYPE_BUFFER,
} stream_type_t;

/**
 * @brief Represents a data stream that can be read from a file or a memory buffer.
 */
typedef struct {
    stream_type_t type;
    union {
        struct {
            lv_fs_file_t * file;
        } file_src;
        struct {
            const void * data;
            size_t size;
            size_t position;
        } buffer_src;
    } src;
} stream_t;




// --- Private Data Structures (Direct mapping of the file format) ---

#pragma pack(push, 1)

/**
 * @brief Represents the V5 header structure at the beginning of the binary font file.
 */
typedef struct {
    uint32_t magic;
    uint32_t version;
    float    size;
    float    xAdvance;
    float    ascender;
    float    descender;
    uint32_t units_per_em;
    int16_t  underline_position;
    int16_t  underline_thickness;
    uint32_t glyph_count;
    uint32_t ult_offset; // Unicode Lookup Table offset
    uint32_t gmt_offset; // Glyph Metadata Table offset
} phys_font_header_t;

/**
 * @brief Represents a single entry in the Glyph Metadata Table (GMT).
 */
typedef struct {
    float    xAdvance;
    int16_t  bbox_xmin, bbox_ymin, bbox_xmax, bbox_ymax;
    uint32_t geometry_offset;
} phys_glyph_metadata_entry_t;

#pragma pack(pop)

/**
 * @brief The internal definition of the font handle. This structure is opaque to the user.
 */
struct lv_ambiq_ttf_handle_t {
    stream_t stream;
    phys_font_header_t header;
    uint32_t * unicode_lookup_table;      // The ULT is always loaded into RAM.
    lv_fs_file_t * file_handle_to_close; // If loaded from a file, this stores the handle for cleanup.
    lv_cache_t * metric_cache;           // L1 Cache for metrics
    lv_cache_t * shape_cache;            // L1 Cache for raw shape data
};

typedef struct {
    uint32_t glyph_index;
    float    xAdvance;
    int16_t  bbox_xmin;
    int16_t  bbox_ymin;
    int16_t  bbox_xmax;
    int16_t  bbox_ymax;
    uint32_t glyph_data_offset;
    uint32_t glyph_data_length;
} phys_glyph_metrics_t;

// --- Cache Node Definitions ---
typedef struct {
    uint32_t glyph_index;                 // Key
    phys_glyph_metrics_t * metrics;       // Cached metrics allocated via font_data_malloc
} phys_metric_cache_node_t;

typedef struct {
    uint32_t glyph_index;                 // Key
    void * shape_data;                    // Cached raw file data allocated via font_data_malloc
} phys_shape_cache_node_t;




/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool adapter_get_glyph_dsc_cb(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter,
                                     uint32_t unicode_letter_next);
static const void * adapter_get_glyph_bitmap_cb(lv_font_glyph_dsc_t * g_dsc, lv_draw_buf_t * draw_buf);
static void adapter_release_glyph_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc);

static bool bitmap_cache_create_cb(bitmap_cache_node_t * node, void * user_data);
static void bitmap_cache_free_cb(bitmap_cache_node_t * node, void * user_data);
static lv_cache_compare_res_t bitmap_cache_compare_cb(const bitmap_cache_node_t * lhs,
                                                      const bitmap_cache_node_t * rhs);

// Forward declarations for metric cache callbacks
static bool metric_cache_create_cb(phys_metric_cache_node_t * node, void * user_data);
static void metric_cache_free_cb(phys_metric_cache_node_t * node, void * user_data);
static lv_cache_compare_res_t metric_cache_compare_cb(const phys_metric_cache_node_t * a,
                                                      const phys_metric_cache_node_t * b);

// Forward declarations for shape cache callbacks
static bool shape_cache_create_cb(phys_shape_cache_node_t * node, void * user_data);
static void shape_cache_free_cb(phys_shape_cache_node_t * node, void * user_data);
static lv_cache_compare_res_t shape_cache_compare_cb(const phys_shape_cache_node_t * a,
                                                     const phys_shape_cache_node_t * b);

// Forward declarations for static stream functions
static void stream_init_from_buffer(stream_t * stream, const void * buffer, size_t size);
static void stream_init_from_file(stream_t * stream, lv_fs_file_t * file);
static size_t stream_read(stream_t * stream, void * data, size_t to_read);
static void stream_seek(stream_t * stream, size_t position);

// Forward declarations for internal font functions
static lv_result_t phys_font_get_glyph_info_by_index(lv_ambiq_ttf_handle_t * font, uint32_t glyph_index,
                                                     phys_glyph_metrics_t * metrics_out);
static lv_result_t phys_font_get_glyph_info_nocache(lv_ambiq_ttf_handle_t * font, uint32_t glyph_index,
                                                    phys_glyph_metrics_t * metrics_out);
static NEMA_VG_PATH_HANDLE phys_font_create_path_from_raw(const void * raw_data);

lv_result_t phys_font_get_glyph_info(lv_ambiq_ttf_handle_t * font, uint32_t unicode,
                                     phys_glyph_metrics_t * metrics_out);
NEMA_VG_PATH_HANDLE phys_font_create_path(lv_ambiq_ttf_handle_t * font, uint32_t glyph_index);
void phys_font_free_path(NEMA_VG_PATH_HANDLE path);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_font_t * lv_ambiq_ttf_create(lv_ambiq_ttf_handle_t * nema_font_handle, int32_t font_size, uint32_t bitmap_cache_size)
{
    if(!nema_font_handle || font_size <= 0) {
        LV_LOG_ERROR("Invalid arguments for lv_ambiq_ttf_create.");
        return NULL;
    }

    lv_font_t * new_font = (lv_font_t *)lv_malloc_zeroed(sizeof(lv_font_t));
    if(new_font == NULL) {
        LV_LOG_ERROR("Failed to allocate memory for lv_font_t.");
        return NULL;
    }

    adapter_ctx_t * dsc = lv_malloc_zeroed(sizeof(adapter_ctx_t));
    if(dsc == NULL) {
        LV_LOG_ERROR("Failed to allocate memory for adapter_ctx_t.");
        lv_free(new_font);
        return NULL;
    }

    dsc->nema_font = nema_font_handle;
    dsc->font_size = font_size;

    int32_t unscaled_line_height = (int32_t)ceilf(nema_font_handle->header.ascender - nema_font_handle->header.descender);
    int32_t unscaled_base_line = (int32_t)ceilf(-nema_font_handle->header.descender);

    float scale = 1.0f;
    if(unscaled_line_height > 0) {
        scale = (float)font_size / (float)unscaled_line_height;
    }
    dsc->scale = scale;

    if(bitmap_cache_size > 0) {
        dsc->bitmap_cache = lv_cache_create(&lv_cache_class_lru_rb_count, sizeof(bitmap_cache_node_t),
                                            bitmap_cache_size,
        (lv_cache_ops_t) {
            .compare_cb = (lv_cache_compare_cb_t)bitmap_cache_compare_cb,
            .create_cb = (lv_cache_create_cb_t)bitmap_cache_create_cb,
            .free_cb = (lv_cache_free_cb_t)bitmap_cache_free_cb,
        });
        lv_cache_set_name(dsc->bitmap_cache, "AMBIQ_TTF_BITMAP");
    }
    else {
        dsc->bitmap_cache = NULL;
    }

    new_font->dsc = dsc;
    new_font->get_glyph_dsc = adapter_get_glyph_dsc_cb;
    new_font->get_glyph_bitmap = adapter_get_glyph_bitmap_cb;
    new_font->release_glyph = adapter_release_glyph_cb;
    new_font->kerning = LV_FONT_KERNING_NONE;

    new_font->line_height = font_size;
    new_font->base_line = (int32_t)roundf(unscaled_base_line * scale);
    new_font->underline_position = (int8_t)roundf(nema_font_handle->header.underline_position * scale);
    float scaled_thickness = nema_font_handle->header.underline_thickness * scale;
    new_font->underline_thickness = (int8_t)fmaxf(1.0f, roundf(scaled_thickness));
    new_font->static_bitmap = 1;

    return new_font;
}

void lv_ambiq_ttf_destroy(lv_font_t * font)
{
    if(!font) return;
    if(font->dsc) {
        adapter_ctx_t * dsc = (adapter_ctx_t *)font->dsc;
        if(dsc->bitmap_cache) {
            lv_cache_destroy(dsc->bitmap_cache, NULL);
        }
        lv_free(dsc);
    }
    lv_free(font);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static bool adapter_get_glyph_dsc_cb(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter,
                                     uint32_t unicode_letter_next)
{
    LV_UNUSED(unicode_letter_next);
    if(!font || !font->dsc || !dsc_out) {
        return false;
    }

    adapter_ctx_t * dsc = (adapter_ctx_t *)font->dsc;

    phys_glyph_metrics_t glyph_info;
    if(phys_font_get_glyph_info(dsc->nema_font, unicode_letter, &glyph_info) != LV_RESULT_OK) {
        return false;
    }

    float scale = dsc->scale;
    dsc_out->adv_w = (uint16_t)roundf(glyph_info.xAdvance * scale);

    if(glyph_info.bbox_xmax > glyph_info.bbox_xmin && glyph_info.bbox_ymax > glyph_info.bbox_ymin) {
        dsc_out->box_w = (uint16_t)roundf((glyph_info.bbox_xmax - glyph_info.bbox_xmin) * scale) + 2;
        dsc_out->box_h = (uint16_t)roundf((glyph_info.bbox_ymax - glyph_info.bbox_ymin) * scale) + 2;
        dsc_out->ofs_x = (int16_t)roundf(glyph_info.bbox_xmin * scale) - 1;
        dsc_out->ofs_y = (int16_t)roundf(glyph_info.bbox_ymin * scale) - 1;
    }
    else {
        dsc_out->box_w = 0;
        dsc_out->box_h = 0;
        dsc_out->ofs_x = 0;
        dsc_out->ofs_y = 0;
    }
    dsc_out->format = LV_FONT_GLYPH_FORMAT_A8;
    dsc_out->is_placeholder = (glyph_info.glyph_index == 0) ? 1 : 0;
    dsc_out->outline_stroke_width = 0;
    dsc_out->gid.index = glyph_info.glyph_index;

    dsc_out->entry = NULL;
    return true;
}

static const void * adapter_get_glyph_bitmap_cb(lv_font_glyph_dsc_t * g_dsc, lv_draw_buf_t * draw_buf)
{
    LV_UNUSED(draw_buf);

    adapter_ctx_t * dsc = (adapter_ctx_t *)g_dsc->resolved_font->dsc;

    bitmap_create_ctx_t ctx = {
        .dsc = dsc,
        .glyph_index = g_dsc->gid.index,
        .box_w = g_dsc->box_w,
        .box_h = g_dsc->box_h
    };

    if(dsc->bitmap_cache == NULL) {
        // Cache is disabled, we cannot return a persistent buffer pointer directly without caching
        // So a cache size of > 0 is mandatory for TTF rendering in LVGL
        LV_LOG_WARN("bitmap_cache = 0 is not supported now, set a non-zero value.");
        return NULL;
    }

    bitmap_cache_node_t search_key = {.glyph_index = g_dsc->gid.index};
    //AM_DEBUG_PIN_SET(DEBUG_PIN_6);
    lv_cache_entry_t * entry = lv_cache_acquire_or_create(dsc->bitmap_cache, &search_key, &ctx);
    if(entry == NULL) {
        return NULL;
    }
    //AM_DEBUG_PIN_CLEAR(DEBUG_PIN_6);

    g_dsc->entry = entry;

    bitmap_cache_node_t * cache_data = lv_cache_entry_get_data(entry);
    const void * glyph_bitmap = cache_data->bitmap->data;

    return glyph_bitmap;
}

static void adapter_release_glyph_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc)
{
    if(g_dsc->entry == NULL) {
        return;
    }
    adapter_ctx_t * dsc = (adapter_ctx_t *)font->dsc;

    bitmap_cache_node_t * cache_data = lv_cache_entry_get_data(g_dsc->entry);
    cache_data->cl_id = nema_get_last_submission_id() + 1;

    lv_cache_release(dsc->bitmap_cache, g_dsc->entry, NULL);
    g_dsc->entry = NULL;
}

static bool bitmap_cache_create_cb(bitmap_cache_node_t * node, void * user_data)
{
    bitmap_create_ctx_t * ctx = (bitmap_create_ctx_t *)user_data;

    if(ctx->box_w <= 0 || ctx->box_h <= 0) {
        node->bitmap = NULL;
        return true; // Space characters
    }

    NEMA_VG_PATH_HANDLE path = phys_font_create_path(ctx->dsc->nema_font, ctx->glyph_index);
    if(!path) {
        LV_LOG_ERROR("Get glyph vector data failed!");
        return false;
    }

    uint32_t bitmap_width = ctx->box_w;
    uint32_t bitmap_height = ctx->box_h;

    lv_draw_buf_t * bitmap_buffer = lv_draw_buf_create_ex(&(LV_GLOBAL_DEFAULT()->font_draw_buf_handlers), bitmap_width,
                                                          bitmap_height,
                                                          LV_COLOR_FORMAT_L8, bitmap_width);
    if(bitmap_buffer == NULL) {
        phys_font_free_path(path);
        LV_LOG_ERROR("Glyph bitmap allocation failed!");
        return false;
    }
    AM_DEBUG_PIN_SET(DEBUG_PIN_5);
    lv_ambiq_vector_path_to_bitmap(path, bitmap_width, bitmap_height, bitmap_buffer->data, ctx->dsc->scale);
    AM_DEBUG_PIN_CLEAR(DEBUG_PIN_5);

    lv_ambiq_blend_mode_clear(NULL);

    phys_font_free_path(path);
    node->bitmap = bitmap_buffer;
    node->cl_id = -1;

    return true;
}

static void bitmap_cache_free_cb(bitmap_cache_node_t * node, void * user_data)
{
    LV_UNUSED(user_data);
    if(node->bitmap) {
        if(nema_get_last_cl_id() >= node->cl_id) {
            lv_draw_buf_destroy(node->bitmap);
        }
        else {
            nema_gc_add(node->bitmap, (void (*)(void *))lv_draw_buf_destroy);
        }
    }
}

static lv_cache_compare_res_t bitmap_cache_compare_cb(const bitmap_cache_node_t * lhs,
                                                      const bitmap_cache_node_t * rhs)
{
    if(lhs->glyph_index != rhs->glyph_index) {
        return lhs->glyph_index > rhs->glyph_index ? 1 : -1;
    }
    return 0;
}

/**
 * @brief Core logic for loading font metadata from an initialized stream.
 * @param font A pointer to an allocated lv_ambiq_ttf_handle_t handle.
 * @return `LV_RESULT_OK` on success, or `LV_RESULT_INVALID` on failure.
 */
static lv_result_t nema_font_load_from_stream(lv_ambiq_ttf_handle_t * font)
{
    if(stream_read(&font->stream, &font->header, sizeof(phys_font_header_t)) != sizeof(phys_font_header_t)) {
        LV_LOG_ERROR("Failed to read font header.");
        return LV_RESULT_INVALID;
    }
    if(font->header.magic != FONT_FILE_MAGIC) {
        LV_LOG_ERROR("Invalid font file magic. Expected 0x%lX, got 0x%lX.", (unsigned long)FONT_FILE_MAGIC,
                     (unsigned long)font->header.magic);
        return LV_RESULT_INVALID;
    }
    if(font->header.version != FONT_FILE_VERSION) {
        LV_LOG_ERROR("Font version mismatch. Expected %d, got %lu.", FONT_FILE_VERSION, (unsigned long)font->header.version);
        return LV_RESULT_INVALID;
    }

    size_t ult_size = font->header.glyph_count * sizeof(uint32_t);
    font->unicode_lookup_table = (uint32_t *)lv_malloc(ult_size);
    if(!font->unicode_lookup_table) {
        LV_LOG_ERROR("Failed to allocate memory for Unicode table (%zu bytes).", ult_size);
        return LV_RESULT_INVALID;
    }
    stream_seek(&font->stream, font->header.ult_offset);
    if(stream_read(&font->stream, font->unicode_lookup_table, ult_size) != ult_size) {
        LV_LOG_ERROR("Failed to read Unicode table.");
        lv_free(font->unicode_lookup_table);
        return LV_RESULT_INVALID;
    }

    return LV_RESULT_OK;
}


// --- Public API Implementation ---

lv_ambiq_ttf_handle_t * lv_ambiq_ttf_load(const char * path, uint32_t metric_cache_size, uint32_t shape_cache_size)
{
    if(!path) {
        LV_LOG_WARN("Invalid arguments.");
        return NULL;
    }
    lv_ambiq_ttf_handle_t * font = (lv_ambiq_ttf_handle_t *)lv_zalloc(sizeof(lv_ambiq_ttf_handle_t));
    if(!font) {
        LV_LOG_ERROR("Out of memory for font handle.");
        return NULL;
    }

    lv_fs_file_t * file_p = (lv_fs_file_t *)lv_malloc(sizeof(lv_fs_file_t));
    if(!file_p) {
        lv_free(font);
        LV_LOG_ERROR("Out of memory for file handle.");
        return NULL;
    }

    lv_fs_res_t res = lv_fs_open(file_p, path, LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK) {
        lv_free(file_p);
        lv_free(font);
        LV_LOG_ERROR("Failed to open font file: %s (error: %d)", path, res);
        return NULL;
    }

    font->file_handle_to_close = file_p;
    stream_init_from_file(&font->stream, file_p);

    if(nema_font_load_from_stream(font) != LV_RESULT_OK) {
        lv_ambiq_ttf_unload(font);
        return NULL;
    }

    if(metric_cache_size > 0) {
        font->metric_cache = lv_cache_create(&lv_cache_class_lru_rb_count, sizeof(phys_metric_cache_node_t), metric_cache_size,
        (lv_cache_ops_t) {
            .compare_cb = (lv_cache_compare_cb_t)metric_cache_compare_cb,
            .create_cb = (lv_cache_create_cb_t)metric_cache_create_cb,
            .free_cb = (lv_cache_free_cb_t)metric_cache_free_cb,
        });
        lv_cache_set_name(font->metric_cache, "AMBIQ_NEMA_FONT_METRIC");
    }
    else {
        font->metric_cache = NULL;
    }

    if(shape_cache_size > 0) {
        font->shape_cache = lv_cache_create(&lv_cache_class_lru_rb_count, sizeof(phys_shape_cache_node_t), shape_cache_size,
        (lv_cache_ops_t) {
            .compare_cb = (lv_cache_compare_cb_t)shape_cache_compare_cb,
            .create_cb = (lv_cache_create_cb_t)shape_cache_create_cb,
            .free_cb = (lv_cache_free_cb_t)shape_cache_free_cb,
        });
        lv_cache_set_name(font->shape_cache, "AMBIQ_NEMA_FONT_SHAPE");
    }
    else {
        font->shape_cache = NULL;
    }

    LV_LOG_INFO("Font loaded successfully from: %s", path);
    return font;
}

lv_ambiq_ttf_handle_t * lv_ambiq_ttf_load_from_buffer(const void * buffer, size_t size)
{
    if(!buffer || size == 0) {
        LV_LOG_WARN("Invalid arguments.");
        return NULL;
    }
    lv_ambiq_ttf_handle_t * font = (lv_ambiq_ttf_handle_t *)lv_malloc_zeroed(sizeof(lv_ambiq_ttf_handle_t));
    if(!font) {
        LV_LOG_ERROR("Out of memory for font handle.");
        return NULL;
    }

    stream_init_from_buffer(&font->stream, buffer, size);

    if(nema_font_load_from_stream(font) != LV_RESULT_OK) {
        lv_ambiq_ttf_unload(font);
        return NULL;
    }

    LV_LOG_INFO("Font loaded successfully from buffer.");
    return font;
}

void lv_ambiq_ttf_unload(lv_ambiq_ttf_handle_t * font)
{
    if(!font) return;
    if(font->metric_cache) lv_cache_destroy(font->metric_cache, NULL);
    if(font->shape_cache) lv_cache_destroy(font->shape_cache, NULL);
    if(font->file_handle_to_close) {
        lv_fs_close(font->file_handle_to_close);
        lv_free(font->file_handle_to_close);
    }
    if(font->unicode_lookup_table) lv_free(font->unicode_lookup_table);
    lv_free(font);
}

bool lv_ambiq_ttf_is_valid(const lv_ambiq_ttf_handle_t * font)
{
    if(!font) return false;
    return font->header.magic == FONT_FILE_MAGIC && font->header.version == FONT_FILE_VERSION;
}

static lv_result_t phys_font_get_glyph_info_nocache(lv_ambiq_ttf_handle_t * font, uint32_t glyph_index,
                                                    phys_glyph_metrics_t * metrics_out)
{
    phys_glyph_metadata_entry_t meta_entries[2];
    size_t gmt_offset = font->header.gmt_offset + glyph_index * sizeof(phys_glyph_metadata_entry_t);
    stream_seek(&font->stream, gmt_offset);
    if(stream_read(&font->stream, meta_entries, sizeof(meta_entries)) != sizeof(meta_entries)) {
        LV_LOG_ERROR("Failed to read glyph metadata for index %lu", (unsigned long)glyph_index);
        return LV_RESULT_INVALID;
    }

    metrics_out->xAdvance = meta_entries[0].xAdvance;
    metrics_out->bbox_xmin = meta_entries[0].bbox_xmin;
    metrics_out->bbox_ymin = meta_entries[0].bbox_ymin;
    metrics_out->bbox_xmax = meta_entries[0].bbox_xmax;
    metrics_out->bbox_ymax = meta_entries[0].bbox_ymax;
    metrics_out->glyph_index = glyph_index;
    metrics_out->glyph_data_offset = meta_entries[0].geometry_offset;
    metrics_out->glyph_data_length = meta_entries[1].geometry_offset - meta_entries[0].geometry_offset;

    return LV_RESULT_OK;
}

static NEMA_VG_PATH_HANDLE phys_font_create_path_from_raw(const void * raw_data)
{
    if(!raw_data) return NULL;

    uint32_t coord_count = *((uint32_t *)raw_data);
    uint32_t seg_len_bytes = *(((uint32_t *)raw_data) + 1);

    uint32_t float_data_len_bytes = coord_count * sizeof(float);
    uint32_t final_length = 8 + float_data_len_bytes + seg_len_bytes;

    void * geometry_block = lv_malloc(final_length);
    if(!geometry_block) {
        LV_LOG_ERROR("Out of memory for geometry block (%lu bytes)", (unsigned long)final_length);
        return NULL;
    }

    *((uint32_t *)geometry_block) = float_data_len_bytes;
    *(((uint32_t *)geometry_block) + 1) = seg_len_bytes;

    float * coords_ptr = (float *)((char *)geometry_block + 8);
    uint8_t * segments_ptr = (uint8_t *)((char *)geometry_block + 8 + float_data_len_bytes);

    if(coord_count > 0) {
        int16_t * src_coords = (int16_t *)((const char *)raw_data + 8);
        for(uint32_t i = 0; i < coord_count; ++i) {
            coords_ptr[i] = (float)src_coords[i];
        }
    }

    if(seg_len_bytes > 0) {
        uint8_t * src_segments = (uint8_t *)((const char *)raw_data + 8 + coord_count * sizeof(int16_t));
        memcpy(segments_ptr, src_segments, seg_len_bytes);
    }

    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
    if(path == NULL) {
        LV_LOG_ERROR("path object create failed!");
        return NULL;
    }
    nema_vg_path_set_shape(path, seg_len_bytes, segments_ptr, coord_count, coords_ptr);
    return path;
}

static inline void * font_data_malloc(size_t size)
{
    nema_buffer_t buf;
    buf = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, size);
    return buf.base_virt;
}

static inline void font_data_free(void * buf)
{
    nema_buffer_t nema_buf = {
        .base_virt = buf,
        .base_phys = (uintptr_t)buf,
        .fd = NEMA_MEM_POOL_ASSETS,
        .size = 0,/*not used in nema_buffer_destroy API*/
    };

    nema_buffer_destroy(&nema_buf);
}

static bool metric_cache_create_cb(phys_metric_cache_node_t * node, void * user_data)
{
    lv_ambiq_ttf_handle_t * font = (lv_ambiq_ttf_handle_t *)user_data;
    node->metrics = font_data_malloc(sizeof(phys_glyph_metrics_t));
    if(!node->metrics) return false;

    if(phys_font_get_glyph_info_nocache(font, node->glyph_index, node->metrics) != LV_RESULT_OK) {
        font_data_free(node->metrics);
        node->metrics = NULL;
        return false;
    }
    return true;
}

static void metric_cache_free_cb(phys_metric_cache_node_t * node, void * user_data)
{
    LV_UNUSED(user_data);
    if(node->metrics) {
        font_data_free(node->metrics);
        node->metrics = NULL;
    }
}

static lv_cache_compare_res_t metric_cache_compare_cb(const phys_metric_cache_node_t * a,
                                                      const phys_metric_cache_node_t * b)
{
    if(a->glyph_index == b->glyph_index) return 0;
    return a->glyph_index > b->glyph_index ? 1 : -1;
}


static bool shape_cache_create_cb(phys_shape_cache_node_t * node, void * user_data)
{
    lv_ambiq_ttf_handle_t * font = (lv_ambiq_ttf_handle_t *)user_data;
    phys_glyph_metrics_t metrics;
    if(phys_font_get_glyph_info_by_index(font, node->glyph_index, &metrics) != LV_RESULT_OK) {
        return false;
    }

    if(metrics.glyph_data_length > 0) {
        node->shape_data = font_data_malloc(metrics.glyph_data_length);
        if(!node->shape_data) return false;

        stream_seek(&font->stream, metrics.glyph_data_offset);
        if(stream_read(&font->stream, node->shape_data, metrics.glyph_data_length) != metrics.glyph_data_length) {
            font_data_free(node->shape_data);
            node->shape_data = NULL;
            return false;
        }
    }
    else {
        node->shape_data = NULL;
    }
    return true;
}

static void shape_cache_free_cb(phys_shape_cache_node_t * node, void * user_data)
{
    LV_UNUSED(user_data);
    if(node->shape_data) {
        font_data_free(node->shape_data);
        node->shape_data = NULL;
    }
}

static lv_cache_compare_res_t shape_cache_compare_cb(const phys_shape_cache_node_t * a,
                                                     const phys_shape_cache_node_t * b)
{
    if(a->glyph_index == b->glyph_index) return 0;
    return a->glyph_index > b->glyph_index ? 1 : -1;
}

static lv_result_t phys_font_get_glyph_info_by_index(lv_ambiq_ttf_handle_t * font, uint32_t glyph_index,
                                                     phys_glyph_metrics_t * metrics_out)
{
    if(font->metric_cache) {
        phys_metric_cache_node_t search_key = {.glyph_index = glyph_index};
        lv_cache_entry_t * entry = lv_cache_acquire_or_create(font->metric_cache, &search_key, font);
        if(entry) {
            phys_metric_cache_node_t * data = lv_cache_entry_get_data(entry);
            *metrics_out = *(data->metrics);
            lv_cache_release(font->metric_cache, entry, NULL);
            return LV_RESULT_OK;
        }
    }
    return phys_font_get_glyph_info_nocache(font, glyph_index, metrics_out);
}

lv_result_t phys_font_get_glyph_info(lv_ambiq_ttf_handle_t * font, uint32_t unicode,
                                     phys_glyph_metrics_t * metrics_out)
{
    if(!font || !metrics_out) return LV_RESULT_INVALID;

    int32_t low = 0, high = font->header.glyph_count - 1, found_index = -1;
    while(low <= high) {
        int32_t mid = low + (high - low) / 2;
        if(font->unicode_lookup_table[mid] == unicode) {
            found_index = mid;
            break;
        }
        else if(font->unicode_lookup_table[mid] < unicode) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }

    if(found_index == -1) {
        return LV_RESULT_INVALID; // Not found
    }

    return phys_font_get_glyph_info_by_index(font, (uint32_t)found_index, metrics_out);
}

NEMA_VG_PATH_HANDLE phys_font_create_path(lv_ambiq_ttf_handle_t * font, uint32_t glyph_index)
{
    if(!font) return NULL;

    if(font->shape_cache) {
        phys_shape_cache_node_t search_key = {.glyph_index = glyph_index};
        lv_cache_entry_t * entry = lv_cache_acquire_or_create(font->shape_cache, &search_key, font);
        if(entry) {
            phys_shape_cache_node_t * data = lv_cache_entry_get_data(entry);
            NEMA_VG_PATH_HANDLE path = NULL;
            if(data->shape_data) {
                path = phys_font_create_path_from_raw(data->shape_data);
            }
            lv_cache_release(font->shape_cache, entry, NULL);
            return path;
        }
        return NULL;
    }
    else {
        phys_glyph_metrics_t metrics;
        if(phys_font_get_glyph_info_by_index(font, glyph_index, &metrics) != LV_RESULT_OK) {
            return NULL;
        }

        if(metrics.glyph_data_length == 0)
            return NULL;

        void * raw_data = lv_malloc(metrics.glyph_data_length);
        LV_ASSERT_MALLOC(raw_data);
        if(!raw_data) {
            LV_LOG_ERROR("glyph vector data temp buffer malloc failed!");
            return NULL;
        }

        stream_seek(&font->stream, metrics.glyph_data_offset);
        if(stream_read(&font->stream, raw_data, metrics.glyph_data_length) != metrics.glyph_data_length) {
            lv_free(raw_data);
            return NULL;
        }

        NEMA_VG_PATH_HANDLE path = phys_font_create_path_from_raw(raw_data);
        if(path == NULL) {
            LV_LOG_ERROR("path object create failed!");
        }
        lv_free(raw_data);

        return path;
    }
}

void phys_font_free_path(NEMA_VG_PATH_HANDLE path)
{
    if(!path) return;
    uint32_t seg_size;
    uint32_t data_size;
    uint8_t * seg_array;
    float * data_array;

    lv_ambiq_get_path_vbuf(path, &seg_size, &data_size, &seg_array, &data_array);

    uintptr_t buffer_to_be_free = (uintptr_t)data_array - 8;
    lv_free((void *)buffer_to_be_free);

    nema_vg_path_destroy(path);
}


// --- Stream Implementation ---
static size_t stream_read(stream_t * stream, void * data, size_t to_read)
{
    AM_DEBUG_PIN_SET(DEBUG_PIN_6);

    if(stream->type == FONT_STREAM_TYPE_FILE) {
        uint32_t bytes_read = 0;
        lv_fs_read(stream->src.file_src.file, data, to_read, &bytes_read);

        AM_DEBUG_PIN_CLEAR(DEBUG_PIN_6);

        return bytes_read;
    }
    if(stream->type == FONT_STREAM_TYPE_BUFFER) {
        size_t remaining_bytes = stream->src.buffer_src.size - stream->src.buffer_src.position;
        size_t actual_read_size = (to_read > remaining_bytes) ? remaining_bytes : to_read;
        if(actual_read_size > 0) {

            memcpy(data, (const uint8_t *)stream->src.buffer_src.data + stream->src.buffer_src.position, actual_read_size);
            stream->src.buffer_src.position += actual_read_size;

            AM_DEBUG_PIN_CLEAR(DEBUG_PIN_6);
        }
        return actual_read_size;
    }
    return 0;
}
static void stream_seek(stream_t * stream, size_t position)
{
    if(stream->type == FONT_STREAM_TYPE_FILE) {
        lv_fs_seek(stream->src.file_src.file, position, LV_FS_SEEK_SET);
    }
    else if(stream->type == FONT_STREAM_TYPE_BUFFER) {
        stream->src.buffer_src.position = (position > stream->src.buffer_src.size) ? stream->src.buffer_src.size : position;
    }
}
static void stream_init_from_buffer(stream_t * stream, const void * buffer, size_t size)
{
    stream->type = FONT_STREAM_TYPE_BUFFER;
    stream->src.buffer_src.data = buffer;
    stream->src.buffer_src.size = size;
    stream->src.buffer_src.position = 0;
}
static void stream_init_from_file(stream_t * stream, lv_fs_file_t * file)
{
    stream->type = FONT_STREAM_TYPE_FILE;
    stream->src.file_src.file = file;
}


#endif // LV_USE_AMBIQ_TTF