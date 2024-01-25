/**
 * @file lv_snapshot.h
 *
 */

#ifndef LV_SNAPSHOT_H
#define LV_SNAPSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stddef.h>

#include "../../core/lv_obj.h"

/*********************
 *      DEFINES
 *********************/

#if LV_USE_SNAPSHOT
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Take snapshot for object with its children, alloc the memory needed.
 * @param obj   the object to generate snapshot.
 * @param cf    color format for generated image.
 * @return      a pointer to an image descriptor, or NULL if failed.
 */
lv_draw_buf_t * lv_snapshot_take(lv_obj_t * obj, lv_color_format_t cf);

/**
 * Free the snapshot image returned by @ref lv_snapshot_take
 * It will firstly free the data image takes, then the image descriptor.
 * @param dsc   the image descriptor generated by lv_snapshot_take.
 */
void lv_draw_buf_destroy(lv_draw_buf_t * dsc);

/**
 * Create a draw buffer for object to store the snapshot image.
 */
lv_draw_buf_t * lv_snapshot_draw_buf_create(lv_obj_t * obj, lv_color_format_t cf);

/**
 * Reshape the draw buffer to prepare for take snapshot for obj.
 * This is usually used to check if the existing draw buffer is enough for
 * obj snapshot. If return LV_RESULT_INVALID, you should create a new one.
 *
 * @param draw_buf  the draw buffer to reshape.
 * @param obj       the object to generate snapshot.
 */
lv_result_t lv_snapshot_draw_buf_reshape(lv_obj_t * obj, lv_draw_buf_t * draw_buf);

/**
 * Take snapshot for object with its children, save image info to provided buffer.
 * @param obj       the object to generate snapshot.
 * @param cf        color format for new snapshot image.
 *                  It could differ with cf of `draw_buf` as long as the new cf will fit in.
 * @param draw_buf  the draw buffer to store the image result. It's reshaped automatically.
 * @return          LV_RESULT_OK on success, LV_RESULT_INVALID on error.
 */
lv_result_t lv_snapshot_take_to_buf(lv_obj_t * obj, lv_color_format_t cf, lv_draw_buf_t * draw_buf);

/**
 * Legacy API, use `lv_draw_buf_destroy` instead.
 *
 * Free the snapshot image returned by @ref lv_snapshot_take
 * @param dsc   the image descriptor generated by lv_snapshot_take.
 */
static inline void lv_snapshot_free(lv_image_dsc_t * dsc)
{
    LV_LOG_WARN("Legacy API, use lv_draw_buf_destroy instead.");
    lv_draw_buf_destroy((lv_draw_buf_t *)dsc);
}

/**********************
 *      MACROS
 **********************/
#endif /*LV_USE_SNAPSHOT*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
