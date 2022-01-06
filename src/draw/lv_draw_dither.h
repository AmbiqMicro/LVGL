/**
 * @file lv_draw_dither.h
 *
 */

#ifndef LV_DRAW_DITHER_H
#define LV_DRAW_DITHER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../core/lv_obj_pos.h"


/*********************
 *      DEFINES
 *********************/
#if LV_COLOR_DEPTH < 32 && LV_DRAW_COMPLEX == 1 && LV_DITHER_GRADIENT == 1
#define DITHER_GRADIENT 1
#else
#define DITHER_GRADIENT 0
#endif


/**********************
 *      TYPEDEFS
 **********************/
#if DITHER_GRADIENT
/*A signed error color component*/
typedef struct {
    int8_t r, g, b;
} lv_scolor24_t;

struct _lv_gradient_cache_t;
typedef void (*lv_dither_func_t)(struct _lv_gradient_cache_t * grad, lv_coord_t x, lv_coord_t y, lv_coord_t w);

#endif


/**********************
 *    PROTOTYPES
 **********************/
#if LV_DRAW_COMPLEX
#if DITHER_GRADIENT
LV_ATTRIBUTE_FAST_MEM void lv_dither_none(struct _lv_gradient_cache_t * grad, lv_coord_t x, lv_coord_t y, lv_coord_t w);

LV_ATTRIBUTE_FAST_MEM void lv_dither_ordered_hor(struct _lv_gradient_cache_t * grad, const lv_coord_t xs,
                                                 const lv_coord_t y, const lv_coord_t w);
LV_ATTRIBUTE_FAST_MEM void lv_dither_ordered_ver(struct _lv_gradient_cache_t * grad, const lv_coord_t xs,
                                                 const lv_coord_t y, const lv_coord_t w);

#if LV_DITHER_ERROR_DIFFUSION
LV_ATTRIBUTE_FAST_MEM void lv_dither_fs_hor(struct _lv_gradient_cache_t * grad, const lv_coord_t xs, const lv_coord_t y,
                                            const lv_coord_t w);
LV_ATTRIBUTE_FAST_MEM void lv_dither_fs_ver(struct _lv_gradient_cache_t * grad, const lv_coord_t xs, const lv_coord_t y,
                                            const lv_coord_t w);
#endif /* LV_DITHER_ERROR_DIFFUSION */

#endif /* DITHER_GRADIENT */
#endif


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
