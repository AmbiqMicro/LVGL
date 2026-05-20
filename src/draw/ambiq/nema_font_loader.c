/**
 * @file nema_font_loader.c
 * @brief Implements the loader for the custom V5 binary vector font format.
 */


#include "../../lvgl.h"

#if LV_USE_AMBIQ_TTF

#include "nema_font_loader.h"
#include <string.h>
#include <math.h> // For ceilf
#include "gpu_patch.h"
#include "am_mem.h"

#include "am_debug_pin.h"

// --- Defines ---
#define FONT_FILE_MAGIC 0x4E464F4E
#define FONT_FILE_VERSION 6

// --- I/O Stream Abstraction ---

/**
 * @brief Defines the source type for the font data stream.
 */
typedef enum {
    FONT_STREAM_TYPE_UNKNOWN,
    FONT_STREAM_TYPE_FILE,
    FONT_STREAM_TYPE_BUFFER,
} font_stream_type_t;

/**
 * @brief Represents a data stream that can be read from a file or a memory buffer.
 */
typedef struct {
    font_stream_type_t type;
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
} font_stream_t;

// Forward declarations for static stream functions
static void font_stream_init_from_buffer(font_stream_t * stream, const void * buffer, size_t size);
static void font_stream_init_from_file(font_stream_t * stream, lv_fs_file_t * file);
static size_t font_stream_read(font_stream_t * stream, void * data, size_t to_read);
static void font_stream_seek(font_stream_t * stream, size_t position);


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
} nema_font_header_t;

/**
 * @brief Represents a single entry in the Glyph Metadata Table (GMT).
 */
typedef struct {
    float    xAdvance;
    int16_t  bbox_xmin, bbox_ymin, bbox_xmax, bbox_ymax;
    uint32_t geometry_offset;
} nema_glyph_metadata_entry_t;

#pragma pack(pop)

/**
 * @brief The internal definition of the font handle. This structure is opaque to the user.
 */
struct ambiq_vg_font_t {
    font_stream_t stream;
    nema_font_header_t header;
    uint32_t * unicode_lookup_table;      // The ULT is always loaded into RAM.
    lv_fs_file_t * file_handle_to_close; // If loaded from a file, this stores the handle for cleanup.
};


// --- Internal Core Loading Function ---

/**
 * @brief Core logic for loading font metadata from an initialized stream.
 * @param font A pointer to an allocated ambiq_vg_font_t handle.
 * @return `LV_RESULT_OK` on success, or `LV_RESULT_INVALID` on failure.
 */
static lv_result_t nema_font_load_from_stream(ambiq_vg_font_t * font)
{
    if(font_stream_read(&font->stream, &font->header, sizeof(nema_font_header_t)) != sizeof(nema_font_header_t)) {
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
    font_stream_seek(&font->stream, font->header.ult_offset);
    if(font_stream_read(&font->stream, font->unicode_lookup_table, ult_size) != ult_size) {
        LV_LOG_ERROR("Failed to read Unicode table.");
        lv_free(font->unicode_lookup_table);
        return LV_RESULT_INVALID;
    }

    return LV_RESULT_OK;
}


// --- Public API Implementation ---

lv_result_t nema_font_load(const char * path, ambiq_vg_font_t ** font_out)
{
    if(!path || !font_out) {
        LV_LOG_WARN("Invalid arguments.");
        return LV_RESULT_INVALID;
    }
    ambiq_vg_font_t * font = (ambiq_vg_font_t *)lv_zalloc(sizeof(ambiq_vg_font_t));
    if(!font) {
        LV_LOG_ERROR("Out of memory for font handle.");
        return LV_RESULT_INVALID;
    }

    lv_fs_file_t * file_p = (lv_fs_file_t *)lv_malloc(sizeof(lv_fs_file_t));
    if(!file_p) {
        lv_free(font);
        LV_LOG_ERROR("Out of memory for file handle.");
        return LV_RESULT_INVALID;
    }

    lv_fs_res_t res = lv_fs_open(file_p, path, LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK) {
        lv_free(file_p);
        lv_free(font);
        LV_LOG_ERROR("Failed to open font file: %s (error: %d)", path, res);
        return LV_RESULT_INVALID;
    }

    font->file_handle_to_close = file_p;
    font_stream_init_from_file(&font->stream, file_p);

    if(nema_font_load_from_stream(font) != LV_RESULT_OK) {
        nema_font_unload(font);
        *font_out = NULL;
        return LV_RESULT_INVALID;
    }

    *font_out = font;
    LV_LOG_INFO("Font loaded successfully from: %s", path);
    return LV_RESULT_OK;
}

lv_result_t nema_font_load_from_buffer(const void * buffer, size_t size, ambiq_vg_font_t ** font_out)
{
    if(!buffer || size == 0 || !font_out) {
        LV_LOG_WARN("Invalid arguments.");
        return LV_RESULT_INVALID;
    }
    ambiq_vg_font_t * font = (ambiq_vg_font_t *)lv_zalloc(sizeof(ambiq_vg_font_t));
    if(!font) {
        LV_LOG_ERROR("Out of memory for font handle.");
        return LV_RESULT_INVALID;
    }

    font_stream_init_from_buffer(&font->stream, buffer, size);

    if(nema_font_load_from_stream(font) != LV_RESULT_OK) {
        nema_font_unload(font);
        *font_out = NULL;
        return LV_RESULT_INVALID;
    }

    *font_out = font;
    LV_LOG_INFO("Font loaded successfully from buffer.");
    return LV_RESULT_OK;
}

void nema_font_unload(ambiq_vg_font_t * font)
{
    if(!font) return;
    if(font->file_handle_to_close) {
        lv_fs_close(font->file_handle_to_close);
        lv_free(font->file_handle_to_close);
    }
    if(font->unicode_lookup_table) lv_free(font->unicode_lookup_table);
    lv_free(font);
}

bool nema_font_is_valid(const ambiq_vg_font_t * font)
{
    if(!font) return false;
    return font->header.magic == FONT_FILE_MAGIC && font->header.version == FONT_FILE_VERSION;
}

lv_result_t nema_font_get_metrics(const ambiq_vg_font_t * font, ambiq_vg_font_metrics_t * metrics_out)
{
    if(!font || !metrics_out) {
        LV_LOG_WARN("Invalid arguments.");
        return LV_RESULT_INVALID;
    }
    metrics_out->line_height = (int32_t)ceilf(font->header.ascender - font->header.descender);
    metrics_out->base_line = (int32_t)ceilf(-font->header.descender);
    metrics_out->underline_position = font->header.underline_position;
    metrics_out->underline_thickness = font->header.underline_thickness;
    return LV_RESULT_OK;
}

lv_result_t nema_font_get_glyph_info(ambiq_vg_font_t * font, uint32_t unicode,
                                     ambiq_vg_glyph_metrics_t * metrics_out)
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

    // Read the metadata for the found glyph and the next one to calculate data length
    nema_glyph_metadata_entry_t meta_entries[2];
    size_t gmt_offset = font->header.gmt_offset + found_index * sizeof(nema_glyph_metadata_entry_t);
    font_stream_seek(&font->stream, gmt_offset);
    if(font_stream_read(&font->stream, meta_entries, sizeof(meta_entries)) != sizeof(meta_entries)) {
        LV_LOG_ERROR("Failed to read glyph metadata for index %ld", (long)found_index);
        return LV_RESULT_INVALID;
    }

    const nema_glyph_metadata_entry_t * meta = &meta_entries[0];
    const nema_glyph_metadata_entry_t * next_meta = &meta_entries[1];

    metrics_out->xAdvance = meta->xAdvance;
    metrics_out->bbox_xmin = meta->bbox_xmin;
    metrics_out->bbox_ymin = meta->bbox_ymin;
    metrics_out->bbox_xmax = meta->bbox_xmax;
    metrics_out->bbox_ymax = meta->bbox_ymax;
    metrics_out->glyph_index = found_index;
    metrics_out->glyph_data_offset = meta->geometry_offset;
    metrics_out->glyph_data_length = next_meta->geometry_offset - meta->geometry_offset;

    return LV_RESULT_OK;
}

NEMA_VG_PATH_HANDLE nema_font_get_glyph_shape_from_info(ambiq_vg_font_t * font, uint32_t offset, uint32_t length)
{
    if(!font || length == 0) {
        return NULL;
    }

    void * temp_block = lv_malloc(length);
    if(!temp_block) {
        LV_LOG_ERROR("Out of memory for temp geometry block (%lu bytes)", (unsigned long)length);
        return NULL;
    }

    font_stream_seek(&font->stream, offset);
    if(font_stream_read(&font->stream, temp_block, length) != length) {
        LV_LOG_ERROR("Failed to read geometry block at offset %lu", (unsigned long)offset);
        lv_free(temp_block);
        return NULL;
    }

    uint32_t coord_count = *((uint32_t *)temp_block);
    uint32_t seg_len_bytes = *(((uint32_t *)temp_block) + 1);

    uint32_t float_data_len_bytes = coord_count * sizeof(float);
    uint32_t final_length = 8 + float_data_len_bytes + seg_len_bytes;

    void * geometry_block = lv_malloc(final_length);
    if(!geometry_block) {
        LV_LOG_ERROR("Out of memory for geometry block (%lu bytes)", (unsigned long)final_length);
        lv_free(temp_block);
        return NULL;
    }

    *((uint32_t *)geometry_block) = float_data_len_bytes;
    *(((uint32_t *)geometry_block) + 1) = seg_len_bytes;

    float * coords_ptr = (float *)((char *)geometry_block + 8);
    uint8_t * segments_ptr = (uint8_t *)((char *)geometry_block + 8 + float_data_len_bytes);

    if(coord_count > 0) {
        int16_t * src_coords = (int16_t *)((char *)temp_block + 8);
        for(uint32_t i = 0; i < coord_count; ++i) {
            coords_ptr[i] = (float)src_coords[i];
        }
    }

    if(seg_len_bytes > 0) {
        uint8_t * src_segments = (uint8_t *)((char *)temp_block + 8 + coord_count * sizeof(int16_t));
        memcpy(segments_ptr, src_segments, seg_len_bytes);
    }

    lv_free(temp_block);

    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
    nema_vg_path_set_shape(path, seg_len_bytes, segments_ptr, coord_count, coords_ptr);
    return path;
}

void nema_font_free_glyph_shape(NEMA_VG_PATH_HANDLE path)
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
static size_t font_stream_read(font_stream_t * stream, void * data, size_t to_read)
{
    if(stream->type == FONT_STREAM_TYPE_FILE) {
        uint32_t bytes_read = 0;
        lv_fs_read(stream->src.file_src.file, data, to_read, &bytes_read);
        return bytes_read;
    }
    if(stream->type == FONT_STREAM_TYPE_BUFFER) {
        size_t remaining_bytes = stream->src.buffer_src.size - stream->src.buffer_src.position;
        size_t actual_read_size = (to_read > remaining_bytes) ? remaining_bytes : to_read;
        if(actual_read_size > 0) {

            AM_DEBUG_PIN_SET(DEBUG_PIN_6);

            memcpy(data, (const uint8_t *)stream->src.buffer_src.data + stream->src.buffer_src.position, actual_read_size);
            stream->src.buffer_src.position += actual_read_size;

            AM_DEBUG_PIN_CLEAR(DEBUG_PIN_6);
        }
        return actual_read_size;
    }
    return 0;
}
static void font_stream_seek(font_stream_t * stream, size_t position)
{
    if(stream->type == FONT_STREAM_TYPE_FILE) {
        lv_fs_seek(stream->src.file_src.file, position, LV_FS_SEEK_SET);
    }
    else if(stream->type == FONT_STREAM_TYPE_BUFFER) {
        stream->src.buffer_src.position = (position > stream->src.buffer_src.size) ? stream->src.buffer_src.size : position;
    }
}
static void font_stream_init_from_buffer(font_stream_t * stream, const void * buffer, size_t size)
{
    stream->type = FONT_STREAM_TYPE_BUFFER;
    stream->src.buffer_src.data = buffer;
    stream->src.buffer_src.size = size;
    stream->src.buffer_src.position = 0;
}
static void font_stream_init_from_file(font_stream_t * stream, lv_fs_file_t * file)
{
    stream->type = FONT_STREAM_TYPE_FILE;
    stream->src.file_src.file = file;
}

#endif