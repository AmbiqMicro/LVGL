//*****************************************************************************
//
// Copyright (c) 2025, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

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
#include "../lv_draw_private.h"
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
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the rectangle
 */
void lv_draw_ambiq_fill(lv_draw_task_t * t, const lv_draw_fill_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw border with AMBIQ GPU render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the rectangle
 */
void lv_draw_ambiq_border(lv_draw_task_t * t, const lv_draw_border_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw box shadow with AMBIQ GPU render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the rectangle for which the box shadow should be drawn
 */
void lv_draw_ambiq_box_shadow(lv_draw_task_t * t, const lv_draw_box_shadow_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw an image with AMBIQ GPU render. It handles image decoding, tiling, transformations, and recoloring.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the image
 */
void lv_draw_ambiq_image(lv_draw_task_t * t, const lv_draw_image_dsc_t * draw_dsc,
                         const lv_area_t * coords);

/**
 * Draw a label with AMBIQ GPU render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the label
 */
void lv_draw_ambiq_label(lv_draw_task_t * t, const lv_draw_label_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw a letter with AMBIQ GPU render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the letter
 */
void lv_draw_ambiq_letter(lv_draw_task_t * t, const lv_draw_letter_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw an arc with AMBIQ GPU render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the arc
 */
void lv_draw_ambiq_arc(lv_draw_task_t * t, const lv_draw_arc_dsc_t * dsc, const lv_area_t * coords);

/**
 * Draw a line with AMBIQ GPU render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 */
void lv_draw_ambiq_line(lv_draw_task_t * t, const lv_draw_line_dsc_t * dsc);

/**
 * Blend a layer with AMBIQ GPU render
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the layer
 */
void lv_draw_ambiq_layer(lv_draw_task_t * t, const lv_draw_image_dsc_t * draw_dsc, const lv_area_t * coords);

/**
 * Draw a triangle with AMBIQ GPU render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 */
void lv_draw_ambiq_triangle(lv_draw_task_t * t, const lv_draw_triangle_dsc_t * dsc);

/**
 * Mask out a rectangle with radius from a current layer
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 * @param coords        the coordinates of the mask
 */
void lv_draw_ambiq_mask_rect(lv_draw_task_t * t, const lv_draw_mask_rect_dsc_t * dsc, const lv_area_t * coords);

#if LV_USE_VECTOR_GRAPHIC
/**
 * Draw vector graphics with AMBIQ render.
 * @param draw_task     pointer to a draw task
 * @param dsc           the draw descriptor
 */
void lv_draw_ambiq_vector(lv_draw_task_t * t, const lv_draw_vector_dsc_t * dsc);
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
