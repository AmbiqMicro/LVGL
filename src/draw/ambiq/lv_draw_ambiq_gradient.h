/**
 * @file lv_draw_ambiq_gradient.h
 *
 */

#ifndef LV_DRAW_AMBIQ_GRADIENT_H
#define LV_DRAW_AMBIQ_GRADIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#if LV_USE_DRAW_AMBIQ

#include "nema_graphics.h"
#include "nema_interpolators.h"

/*********************
 *      DEFINES
 *********************/



/**********************
 *      TYPEDEFS
 **********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern void lv_ambiq_gradient_create(int stops_count,float *stops, color_var_t* colors, nema_buffer_t* gradient_buffer);

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

#endif /*LV_DRAW_AMBIQ_GRADIENT_H*/