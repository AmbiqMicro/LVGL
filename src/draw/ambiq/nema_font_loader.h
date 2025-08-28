/**
 * @file nema_font_loader.h
 * @brief A high-performance library to load and access a custom binary vector font format.
 */

#ifndef NEMA_FONT_LOADER_H
#define NEMA_FONT_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../lvgl.h"

#if LV_USE_AMBIQ_TTF

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "nema_vg.h"      // Provides NEMA_VG_PATH_HANDLE


/**********************
 *      TYPEDEFS
 **********************/

/** @brief An opaque handle representing a loaded font instance. */
typedef struct ambiq_vg_font_t ambiq_vg_font_t;

/** @brief A structure to hold the unscaled layout metrics for a single glyph. */
typedef struct {
    float    xAdvance;
    int16_t  bbox_xmin, bbox_ymin, bbox_xmax, bbox_ymax;
    uint32_t glyph_index;
    uint32_t glyph_data_offset;
    uint32_t glyph_data_length;
} ambiq_vg_glyph_metrics_t;

/** @brief A structure to hold the global, unscaled metrics of a font. */
typedef struct {
    int32_t line_height;
    int32_t base_line;
    int16_t underline_position;
    int16_t underline_thickness;
} ambiq_vg_font_metrics_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Loads a binary font from the file system.
 * @param[in] path The path to the font file, prefixed with an LVGL driver letter.
 * @param[out] font_out A pointer to a `ambiq_vg_font_t*` where the handle will be stored.
 * @return `LV_RESULT_OK` on success, or `LV_RESULT_INVALID` on failure.
 */
lv_result_t nema_font_load(const char * path, ambiq_vg_font_t ** font_out);

/**
 * @brief Loads a binary font from a memory buffer.
 * @param[in] buffer A pointer to the memory buffer containing the font data.
 * @param[in] size The size of the buffer in bytes.
 * @param[out] font_out A pointer to a `ambiq_vg_font_t*` where the handle will be stored.
 * @return `LV_RESULT_OK` on success, or `LV_RESULT_INVALID` on failure.
 */
lv_result_t nema_font_load_from_buffer(const void * buffer, size_t size, ambiq_vg_font_t ** font_out);

/**
 * @brief Unloads a font and frees all associated resources.
 * @param[in] font The font handle to unload. It is safe to pass `NULL`.
 */
void nema_font_unload(ambiq_vg_font_t * font);

/**
 * @brief Checks if a loaded font handle is valid and compatible.
 * @param[in] font The font handle to check.
 * @return `true` if valid and compatible, `false` otherwise.
 */
bool nema_font_is_valid(const ambiq_vg_font_t * font);

/**
 * @brief Retrieves the global layout metrics for a loaded font.
 * @param[in] font The loaded font handle.
 * @param[out] metrics_out A pointer to a structure to be filled with the metrics.
 * @return `LV_RESULT_OK` on success, or `LV_RESULT_INVALID` on failure.
 */
lv_result_t nema_font_get_metrics(const ambiq_vg_font_t * font, ambiq_vg_font_metrics_t * metrics_out);

/**

 * @brief Gets the layout metrics for a single glyph (I/O-Free).
 * @param[in] font The loaded font handle.
 * @param[in] unicode The Unicode codepoint of the desired glyph.
 * @param[out] metrics_out A pointer to a structure to be filled with glyph metrics.
 * @return `LV_RESULT_OK` if the glyph is found, or `LV_RESULT_INVALID` if not.
 */
lv_result_t nema_font_get_glyph_info(ambiq_vg_font_t * font, uint32_t unicode,
                                     ambiq_vg_glyph_metrics_t * metrics_out);

/**
 * @brief Gets the renderable shape (path) for a single glyph (Performs I/O).
 * @param[in] font The loaded font handle.
 * @param[in] offset The offset of the glyph data in the font file.
 * @param[in] length The length of the glyph data.
 * @return A valid `NEMA_VG_PATH_HANDLE` on success, or `NULL` on failure.
 */
NEMA_VG_PATH_HANDLE nema_font_get_glyph_shape_from_info(ambiq_vg_font_t * font, uint32_t offset, uint32_t length);

/**
 * @brief Frees a NEMA VG path object created by `nema_font_get_glyph_shape()`.
 * @param[in] path The path handle to free. It is safe to pass `NULL`.
 */
void nema_font_free_glyph_shape(NEMA_VG_PATH_HANDLE path);

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // NEMA_FONT_LOADER_H