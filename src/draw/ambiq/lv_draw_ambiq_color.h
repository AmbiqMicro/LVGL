/**
 * @file lv_draw_ambiq_color.h
 *
 */

#ifndef LV_DRAW_AMBIQ_COLOR_H
#define LV_DRAW_AMBIQ_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#if LV_USE_DRAW_AMBIQ

#include "../../misc/lv_color.h"
#include "nema_graphics.h"


/*********************
 *      DEFINES
 *********************/
#define COLOR_FORMAT_INVALID    (0xffffffffUL)


/**********************
 *      TYPEDEFS
 **********************/
extern nema_tex_format_t lv_ambiq_color_format_map_src(lv_color_format_t lvgl_cf);
extern nema_tex_format_t lv_ambiq_color_format_map_des(lv_color_format_t lvgl_cf);
extern uint32_t lv_ambiq_color_convert(lv_color_t color, lv_opa_t opa);

/**********************
 * GLOBAL PROTOTYPES
 **********************/


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

#endif /*LV_DRAW_AMBIQ_COLOR_H*/