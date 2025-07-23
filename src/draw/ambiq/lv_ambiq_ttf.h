/**
 * @file lv_ambiq_ttf.h
 *
 */

#ifndef LV_AMBIQ_TTF_H
#define LV_AMBIQ_TTF_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"

#if LV_USE_AMBIQ_TTF
#include <stddef.h>
#include "../../font/lv_font.h"
#include "nema_vg.h" // For NEMA_VG_PATH_HANDLE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a font from the specified file path with a given initial size.
 * @param path        the path of the font file, prefixed with an LVGL driver letter.
 * @param font_size   the initial font size in pixels.
 * @return a pointer to the created font object, or NULL on failure.
 */
lv_font_t * lv_ambiq_ttf_create_file(const char * path, int32_t font_size);

/**
 * Create a font from a memory buffer with a given initial size.
 * @param data        a pointer to the buffer containing the font data.
 * @param data_size   the size of the data buffer in bytes.
 * @param font_size   the initial font size in pixels.
 * @return a pointer to the created font object, or NULL on failure.
 */
lv_font_t * lv_ambiq_ttf_create_data(const void * data, size_t data_size, int32_t font_size);

/**
 * Sets or changes the size of the font.
 * @note After changing the size, you may need to invalidate any UI objects
 *       using this font to trigger a redraw with the new metrics.
 * @param font        the font object to modify.
 * @param font_size   the new desired font size in pixels.
 */
void lv_ambiq_ttf_set_size(lv_font_t * font, int32_t font_size);

/**
 * Destroy a font previously created with `lv_ambiq_ttf_create_file` or `lv_ambiq_ttf_create_data`.
 * @param font        the font object to destroy.
 */
void lv_ambiq_ttf_destroy(lv_font_t * font);

/**
 * @brief Calculates and returns the current scale factor for the font.
 * @param font A pointer to the LVGL font object.
 * @return The calculated floating-point scale factor.
 */
float lv_ambiq_ttf_get_scale(const lv_font_t * font);

/**
 * @brief Checks if a given LVGL font is a valid Ambiq TTF font.
 * @param font The font object to check.
 * @return `true` if it is a valid Ambiq TTF font, `false` otherwise.
 */
bool lv_ambiq_ttf_identify(const lv_font_t * font);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_AMBIQ_TTF*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_AMBIQ_TTF_H*/