﻿/**
 * @file lv_draw_sw_gradient.h
 *
 */

#ifndef LV_DRAW_SW_GRADIENT_H
#define LV_DRAW_SW_GRADIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../misc/lv_color.h"
#include "../../misc/lv_style.h"

#if LV_USE_DRAW_SW

/*********************
 *      DEFINES
 *********************/
#if LV_GRADIENT_MAX_STOPS < 2
#error LVGL needs at least 2 stops for gradients. Please increase the LV_GRADIENT_MAX_STOPS
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef lv_color_t lv_grad_color_t;

typedef struct _lv_gradient_cache_t {
    lv_color_t   *  color_map;
    lv_opa_t   *  opa_map;
    uint32_t size;
} lv_grad_t;

/**********************
 *      PROTOTYPES
 **********************/
/** Compute the color in the given gradient and fraction
 *  Gradient are specified in a virtual [0-255] range, so this function scales the virtual range to the given range
 * @param dsc       The gradient descriptor to use
 * @param range     The range to use in computation.
 * @param frac      The current part used in the range. frac is in [0; range]
 */

void /* LV_ATTRIBUTE_FAST_MEM */ lv_gradient_color_calculate(const lv_grad_dsc_t * dsc, int32_t range,
                                                             int32_t frac, lv_grad_color_t * color_out, lv_opa_t * opa_out);

/** Get a gradient cache from the given parameters */
lv_grad_t * lv_gradient_get(const lv_grad_dsc_t * gradient, int32_t w, int32_t h);

/**
 * Clean up the gradient item after it was get with `lv_grad_get_from_cache`.
 * @param grad      pointer to a gradient
 */
void lv_gradient_cleanup(lv_grad_t * grad);

#if LV_USE_DRAW_SW_COMPLEX_GRADIENTS

/**
 * Calculate constants from the given parameters that are used during rendering
 * @param dsc      gradient descriptor
 */
void lv_gradient_linear_setup(lv_grad_dsc_t * dsc);

/**
 * Free up the allocated memory for the gradient calculation
 * @param dsc      gradient descriptor
 */
void lv_gradient_linear_cleanup(lv_grad_dsc_t * dsc);

/**
 * Calculate a line segment of a linear gradient
 * @param dsc       gradient descriptor
 * @param xp        starting point x coordinate in gradient space
 * @param yp        starting point y coordinate in gradient space
 * @param width     width of the line segment in pixels
 * @param result    color buffer for the resulting line segment
 */
void /* LV_ATTRIBUTE_FAST_MEM */ lv_gradient_linear_get_line(lv_grad_dsc_t * dsc, int32_t xp, int32_t yp, int32_t width,
                                                             lv_grad_t * result);

/**
 * Calculate constants from the given parameters that are used during rendering
 * @param dsc      gradient descriptor
 */
void lv_gradient_radial_setup(lv_grad_dsc_t * dsc);

/**
 * Free up the allocated memory for the gradient calculation
 * @param dsc      gradient descriptor
 */
void lv_gradient_radial_cleanup(lv_grad_dsc_t * dsc);

/**
 * Calculate a line segment of a radial gradient
 * @param dsc       gradient descriptor
 * @param xp        starting point x coordinate in gradient space
 * @param yp        starting point y coordinate in gradient space
 * @param width     width of the line segment in pixels
 * @param result    color buffer for the resulting line segment
 */
void /* LV_ATTRIBUTE_FAST_MEM */ lv_gradient_radial_get_line(lv_grad_dsc_t * dsc, int32_t xp, int32_t yp, int32_t width,
                                                             lv_grad_t * result);

/**
 * Calculate constants from the given parameters that are used during rendering
 * @param dsc      gradient descriptor
 */
void lv_gradient_conical_setup(lv_grad_dsc_t * dsc);

/**
 * Free up the allocated memory for the gradient calculation
 * @param dsc      gradient descriptor
 */
void lv_gradient_conical_cleanup(lv_grad_dsc_t * dsc);

/**
 * Calculate a line segment of a conical gradient
 * @param dsc       gradient descriptor
 * @param xp        starting point x coordinate in gradient space
 * @param yp        starting point y coordinate in gradient space
 * @param width     width of the line segment in pixels
 * @param result    color buffer for the resulting line segment
 */
void /* LV_ATTRIBUTE_FAST_MEM */ lv_gradient_conical_get_line(lv_grad_dsc_t * dsc, int32_t xp, int32_t yp,
                                                              int32_t width,
                                                              lv_grad_t * result);

#endif /*LV_USE_DRAW_SW_COMPLEX_GRADIENTS*/

#endif /*LV_USE_DRAW_SW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_GRADIENT_H*/
