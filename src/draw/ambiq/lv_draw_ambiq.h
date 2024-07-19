/**
 * @file lv_draw_ambiq.h
 *
 */

#ifndef LV_DRAW_AMBIQ_H
#define LV_DRAW_AMBIQ_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#if LV_USE_DRAW_AMBIQ


#include "../../misc/lv_area.h"
#include "../../misc/lv_color.h"
#include "../../display/lv_display.h"
#include "../../osal/lv_os.h"

#include "../lv_draw_vector.h"
#include "../lv_draw_triangle.h"
#include "../lv_draw_label.h"
#include "../lv_draw_image.h"
#include "../lv_draw_line.h"
#include "../lv_draw_arc.h"
#include "../lv_draw_private.h"

#include "../../misc/lv_area_private.h"

#include "lv_draw_ambiq_color.h"
#include "lv_draw_ambiq_gradient.h"

#include "am_mcu_apollo.h"

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_raster.h"
#include "nema_graphics.h"
#include "nema_provisional.h"
#include "nema_error.h"
#include "nema_raster.h"
#include "nema_blender.h"
#include "nema_sys_defs.h"
#include "nema_interpolators.h"
#include "nema_matrix3x3.h"
#include "nema_programHW.h"

#if LV_USE_AMBIQ_VG
#include "nema_vg.h"
#include "nema_vg_font.h"
#include "nema_vg_tsvg.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_draw_unit_t base_unit;
    lv_draw_task_t * task_act;
#if LV_USE_OS
    lv_thread_sync_t sync;
    lv_thread_t thread;
    volatile bool inited;
    volatile bool exit_status;
#endif

    //! Record the size of allocated buffer.
    uint32_t total_buffer;

    //! Record total CLs created.
    uint32_t total_cl;

    //! Last CL submittion id, used for cl_wait()
    int32_t last_cl_id;

    //! CL list head
    nema_cmdlist_t* cl_head;

    //! gradient buffer
    nema_buffer_t small_texture_buffer;

    uint32_t small_texture_buffer_size_byte;

} lv_draw_ambiq_unit_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

extern uint32_t nema_enable_aa_flags(uint32_t aa);

/**
 * Initialize the AMBIQ GPU renderer.
 */
void lv_draw_ambiq_init(void);

/**
 * Deinitialize the AMBIQ GPU renderer.
 */
void lv_draw_ambiq_deinit(void);

/**
 * Fill an area using AMBIQ GPU render. Handle gradient and radius.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the rectangle
 */
void lv_draw_ambiq_fill(lv_draw_unit_t * draw_unit, const lv_draw_fill_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw border with AMBIQ GPU render.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the rectangle
 */
void lv_draw_ambiq_border(lv_draw_unit_t * draw_unit, const lv_draw_border_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw box shadow with AMBIQ GPU render.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the rectangle for which the box shadow should be drawn
 */
void lv_draw_ambiq_box_shadow(lv_draw_unit_t * draw_unit, const lv_draw_box_shadow_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw an image with AMBIQ GPU render. It handles image decoding, tiling, transformations, and recoloring.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the image
 */
void lv_draw_ambiq_image(lv_draw_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc,
                      const lv_area_t * coords);

/**
 * Draw a label with AMBIQ GPU render.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the label
 */
void lv_draw_ambiq_label(lv_draw_unit_t * draw_unit, const lv_draw_label_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw an arc with AMBIQ GPU render.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the arc
 */
void lv_draw_ambiq_arc(lv_draw_unit_t * draw_unit, const lv_draw_arc_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw a line with AMBIQ GPU render.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 */
void lv_draw_ambiq_line(lv_draw_unit_t * draw_unit, const lv_draw_line_dsc_t * dsc);

/**
 * Blend a layer with AMBIQ GPU render
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the layer
 */
void lv_draw_ambiq_layer(lv_draw_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc, const lv_area_t * coords);

/**
 * Draw a triangle with AMBIQ GPU render.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 */
void lv_draw_ambiq_triangle(lv_draw_unit_t * draw_unit, const lv_draw_triangle_dsc_t * dsc);

/**
 * Mask out a rectangle with radius from a current layer
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the mask
 */
void lv_draw_ambiq_mask_rect(lv_draw_unit_t * draw_unit, const lv_draw_mask_rect_dsc_t * dsc, const lv_area_t * coords);

/**
 * Used internally to get a transformed are of an image
 * @param draw_unit     pointer to a draw unit
 * @param dest_area     the area to calculate, i.e. get this area from the transformed image
 * @param src_buf       the source buffer
 * @param src_w         source buffer width in pixels
 * @param src_h         source buffer height in pixels
 * @param src_stride    source buffer stride in bytes
 * @param dsc           the draw descriptor
 * @param sup           supplementary data
 * @param cf            color format of the source buffer
 * @param dest_buf      the destination buffer
 */
void lv_draw_ambiq_transform(lv_draw_unit_t * draw_unit, const lv_area_t * dest_area, const void * src_buf,
                          int32_t src_w, int32_t src_h, int32_t src_stride,
                          const lv_draw_image_dsc_t * draw_dsc, const lv_draw_image_sup_t * sup, lv_color_format_t cf, void * dest_buf);

#if LV_USE_VECTOR_GRAPHIC
/**
 * Draw vector graphics with AMBIQ render.
 * @param draw_unit     pointer to a draw unit
 * @param dsc           the draw descriptor
 */
void lv_draw_ambiq_vector(lv_draw_unit_t * draw_unit, const lv_draw_vector_task_dsc_t * dsc);
#endif

/***********************
 * GLOBAL VARIABLES
 ***********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_AMBIQ*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_AMBIQ_H*/
