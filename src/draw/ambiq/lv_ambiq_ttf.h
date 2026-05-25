/**
 * @file lv_ambiq_ttf.h
 * @brief LVGL scalable vector font adapter for Ambiq NemaVG.
 */

#ifndef LV_AMBIQ_TTF_H
#define LV_AMBIQ_TTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../lv_conf_internal.h"

#if LV_USE_AMBIQ_TTF
#include <stddef.h>
#include "../../font/lv_font.h"

/* -----------------------------------------------------------------------------
 * Phase 1: Physical Font Resource Loading
 * ---------------------------------------------------------------------------*/

/**
 * @brief Opaque handle for the underlying physical vector font data.
 */
typedef struct lv_ambiq_ttf_handle_t lv_ambiq_ttf_handle_t;

/**
 * @brief Loads a binary vector font from the file system.
 *
 * This function loads the font metadata (e.g. Unicode lookup table) into RAM.
 * The raw glyph vector coordinates are dynamically read from the file system and
 * cached into a memory pool to reduce FS latency.
 *
 * @param path               The path to the font file, prefixed with an LVGL driver letter.
 * @param metric_cache_size  The number of glyph metrics to cache (e.g. 1024). 0 disables it.
 * @param shape_cache_size   The number of raw vector shapes to cache. 0 disables it.
 *                           Shape data is dynamically allocated from PSRAM using Nema buffer pool.
 * @return                   A pointer to the font handle, or NULL on failure.
 */
lv_ambiq_ttf_handle_t * lv_ambiq_ttf_load(const char * path, uint32_t metric_cache_size, uint32_t shape_cache_size);

/**
 * @brief Loads a binary vector font from a memory buffer.
 *
 * This is optimal for fonts compiled as C arrays or residing in memory-mapped XIP Flash.
 * It utilizes a zero-copy mechanism for reading coordinates directly from the buffer,
 * therefore the L1 cache is disabled automatically to save RAM.
 *
 * @param buffer        Pointer to the memory buffer containing the font data.
 * @param size          Size of the buffer in bytes.
 * @return              A pointer to the font handle, or NULL on failure.
 */
lv_ambiq_ttf_handle_t * lv_ambiq_ttf_load_from_buffer(const void * buffer, size_t size);

/**
 * @brief Unloads a physical font handle and frees all associated resources.
 *
 * This destroys the L1 cache, the Unicode lookup table, and closes any open files.
 *
 * @param handle        The font handle to unload.
 */
void lv_ambiq_ttf_unload(lv_ambiq_ttf_handle_t * handle);

/**
 * @brief Checks if a loaded font handle is valid and compatible.
 * @param handle        The font handle to check.
 * @return `true` if valid and compatible, `false` otherwise.
 */
bool lv_ambiq_ttf_is_valid(const lv_ambiq_ttf_handle_t * handle);

/* -----------------------------------------------------------------------------
 * Phase 2: Logical LVGL Font Instantiation
 * ---------------------------------------------------------------------------*/

/**
 * @brief Creates a scalable LVGL font object mapped to a physical font handle.
 *
 * This function instantiates an `lv_font_t` for a specific pixel size. It creates
 * an L2 (A8 bitmap) cache using LVGL's internal cache engine to store rasterized
 * glyphs, preventing redundant GPU rendering.
 *
 * @param handle        The physical font handle previously loaded.
 * @param font_size     The target font size in pixels (line height).
 * @param bitmap_cache_size    The number of A8 glyph bitmaps to cache in the L2 cache.
 *                      This should roughly match the number of unique characters
 *                      displayed on the screen simultaneously (e.g., 64-256).
 *                      Memory cost = cache_size * (font_size^2) bytes of draw_buf memory.
 * @return              A pointer to the created LVGL font, or NULL on failure.
 */
lv_font_t * lv_ambiq_ttf_create(lv_ambiq_ttf_handle_t * handle, int32_t font_size, uint32_t bitmap_cache_size);

/**
 * @brief Destroys an LVGL font object previously created with `lv_ambiq_ttf_create`.
 *
 * This destroys the L2 (bitmap) cache associated with this specific size,
 * but DOES NOT unload the underlying physical font handle (which can still be
 * used by other sizes).
 *
 * @param font          The LVGL font to destroy.
 */
void lv_ambiq_ttf_destroy(lv_font_t * font);

#endif /*LV_USE_AMBIQ_TTF*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_AMBIQ_TTF_H*/
