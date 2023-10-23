/**
 * @file lv_vg_lite_path.h
 *
 */

#ifndef LV_VG_LITE_PATH_H
#define LV_VG_LITE_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lv_vg_lite_utils.h"

#if LV_USE_DRAW_VG_LITE

/*********************
 *      DEFINES
 *********************/

typedef struct _lv_vg_lite_path_t lv_vg_lite_path_t;
struct _lv_draw_vg_lite_unit_t;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_vg_lite_path_init(struct _lv_draw_vg_lite_unit_t * unit);

void lv_vg_lite_path_deinit(struct _lv_draw_vg_lite_unit_t * unit);

lv_vg_lite_path_t * lv_vg_lite_path_get(struct _lv_draw_vg_lite_unit_t * unit, vg_lite_format_t data_format);

void lv_vg_lite_path_drop(struct _lv_draw_vg_lite_unit_t * unit, lv_vg_lite_path_t * path);

void lv_vg_lite_path_reset(lv_vg_lite_path_t * path, vg_lite_format_t data_format);

void lv_vg_lite_path_set_bonding_box_area(lv_vg_lite_path_t * path, const lv_area_t * area);

void lv_vg_lite_path_set_bonding_box(lv_vg_lite_path_t * path,
                                     float min_x, float min_y,
                                     float max_x, float max_y);
void lv_vg_lite_path_set_quality(lv_vg_lite_path_t * path, vg_lite_quality_t quality);

vg_lite_path_t * lv_vg_lite_path_get_path(lv_vg_lite_path_t * path);

void lv_vg_lite_path_move_to(lv_vg_lite_path_t * path,
                             float x, float y);

void lv_vg_lite_path_line_to(lv_vg_lite_path_t * path,
                             float x, float y);

void lv_vg_lite_path_quad_to(lv_vg_lite_path_t * path,
                             float cx, float cy,
                             float x, float y);

void lv_vg_lite_path_cubic_to(lv_vg_lite_path_t * path,
                              float cx1, float cy1,
                              float cx2, float cy2,
                              float x, float y);

void lv_vg_lite_path_close(lv_vg_lite_path_t * path);

void lv_vg_lite_path_end(lv_vg_lite_path_t * path);

void lv_vg_lite_path_append_rect(lv_vg_lite_path_t * path,
                                 float x, float y,
                                 float w, float h,
                                 float rx, float ry);

void lv_vg_lite_path_append_circle(lv_vg_lite_path_t * path,
                                   float cx, float cy,
                                   float rx, float ry);

void lv_vg_lite_path_append_arc_right_angle(lv_vg_lite_path_t * path,
                                            float start_x, float start_y,
                                            float center_x, float center_y,
                                            float end_x, float end_y);

void lv_vg_lite_path_append_arc_acute_angle(lv_vg_lite_path_t * path,
                                            float start_x, float start_y,
                                            float center_x, float center_y,
                                            float end_x, float end_y);

void lv_vg_lite_path_append_arc_round(lv_vg_lite_path_t * path,
                                      float cx, float cy,
                                      float start_angle, float end_angle,
                                      float radius,
                                      float width,
                                      bool rounded);

void lv_vg_lite_path_append_arc(lv_vg_lite_path_t * path,
                                float cx, float cy,
                                float radius,
                                float start_angle,
                                float sweep,
                                bool pie);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_VG_LITE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_VG_LITE_PATH_H*/
