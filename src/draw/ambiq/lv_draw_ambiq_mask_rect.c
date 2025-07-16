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
 * @file lv_draw_ambiq_mask_rect.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../misc/lv_area_private.h"
#include "../lv_draw_mask_private.h"
#include "../lv_draw_private.h"
#if LV_USE_DRAW_AMBIQ

#include "../../misc/lv_math.h"
#include "../../misc/lv_log.h"
#include "../../stdlib/lv_mem.h"
#include "../../stdlib/lv_string.h"
#include "lv_draw_ambiq.h"
#include "lv_draw_ambiq_private.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_ambiq_mask_rect(lv_draw_task_t * t, const lv_draw_mask_rect_dsc_t * dsc, const lv_area_t * coords)
{
    LV_UNUSED(coords);

    lv_area_t draw_area;
    if(!lv_area_intersect(&draw_area, &dsc->area, &t->clip_area)) {
        return;
    }

    lv_layer_t * target_layer = t->target_layer;
    lv_area_t * buf_area = &target_layer->buf_area;
    lv_area_t clear_area;
    lv_draw_ambiq_unit_t * unit = (lv_draw_ambiq_unit_t *)t->draw_unit;

    if(!dsc->keep_outside) {
        /* set the blend mode to SRC*/
        lv_ambiq_set_blend_fill(unit, NEMA_BL_SRC);
        nema_set_raster_color(0x00000000);

        /*Clear the top part*/
        lv_area_set(&clear_area, t->clip_area.x1, t->clip_area.y1, t->clip_area.x2,
                    dsc->area.y1 - 1);
        lv_area_move(&clear_area, -buf_area->x1, -buf_area->y1);
        nema_raster_rect(clear_area.x1, clear_area.y1, lv_area_get_width(&clear_area), lv_area_get_height(&clear_area));

        /*Clear the bottom part*/
        lv_area_set(&clear_area, t->clip_area.x1, dsc->area.y2 + 1, t->clip_area.x2,
                    t->clip_area.y2);
        lv_area_move(&clear_area, -buf_area->x1, -buf_area->y1);
        nema_raster_rect(clear_area.x1, clear_area.y1, lv_area_get_width(&clear_area), lv_area_get_height(&clear_area));

        /*Clear the left part*/
        lv_area_set(&clear_area, t->clip_area.x1, dsc->area.y1, dsc->area.x1 - 1, dsc->area.y2);
        lv_area_move(&clear_area, -buf_area->x1, -buf_area->y1);
        nema_raster_rect(clear_area.x1, clear_area.y1, lv_area_get_width(&clear_area), lv_area_get_height(&clear_area));

        /*Clear the right part*/
        lv_area_set(&clear_area, dsc->area.x2 + 1, dsc->area.y1, t->clip_area.x2, dsc->area.y2);
        lv_area_move(&clear_area, -buf_area->x1, -buf_area->y1);
        nema_raster_rect(clear_area.x1, clear_area.y1, lv_area_get_width(&clear_area), lv_area_get_height(&clear_area));
    }

    /*Get the clamped radius*/
    int32_t radius = dsc->radius;
    int32_t short_side = LV_MIN(lv_area_get_width(&dsc->area), lv_area_get_height(&dsc->area));
    if(radius > short_side >> 1) radius = short_side >> 1;



    /*transformation matrix for texture mapping*/
    nema_matrix3x3_t m;

    /* bind the stencil buffer to TEX1*/

    lv_result_t res = lv_draw_ambiq_stencil_buffer_adjust(unit, radius, radius);
    if(res != LV_RESULT_OK) {
        LV_LOG_ERROR("Failed to allocate memory for stencil buffer");
        return;
    }
    nema_bind_tex(NEMA_TEX1,
                  (uintptr_t)unit->stencil_buffer->data,
                  unit->stencil_buffer->header.w,
                  unit->stencil_buffer->header.h,
                  NEMA_L8,
                  unit->stencil_buffer->header.stride,
                  NEMA_FILTER_BL);

    /* set the blend mode to SRC*/
    lv_ambiq_blend_mode_change(unit,  NEMA_BL_SRC, NEMA_TEX1, NEMA_NOTEX, NEMA_NOTEX, false);
    nema_set_clip_temp(0, 0, radius, radius);

    /* draw the mask to this buffer*/
    nema_set_raster_color(0x00000000);
    nema_raster_rect(0, 0, radius, radius);
    nema_set_raster_color(0xffffffff);
    nema_raster_stroked_arc_aa(0, radius, radius * 0.5f, radius, 270.f, 360.f);

    nema_set_clip_pop();

    nema_bind_tex(NEMA_TEX1,
                  (uintptr_t)unit->stencil_buffer->data,
                  unit->stencil_buffer->header.w,
                  unit->stencil_buffer->header.h,
                  NEMA_A8,
                  unit->stencil_buffer->header.stride,
                  NEMA_FILTER_BL);
    lv_ambiq_blend_mode_change(unit, NEMA_BL_DST_IN, NEMA_TEX0, NEMA_TEX1, NEMA_NOTEX, false);
    nema_set_tex_color(0xffffffff);

    lv_area_t blend_area;
    lv_area_t raster_area;
    lv_area_t clip_raster_area;
    int32_t layer_buf_start_x = target_layer->buf_area.x1;
    int32_t layer_buf_start_y = target_layer->buf_area.y1;

    /*Top right corner*/
    blend_area.x2 = dsc->area.x2;
    blend_area.x1 = dsc->area.x2 - radius + 1;
    blend_area.y1 = dsc->area.y1;
    blend_area.y2 = dsc->area.y1 + radius - 1;
    lv_area_copy(&raster_area, &blend_area);

    if(lv_area_intersect(&clip_raster_area, &raster_area, &t->clip_area)) {

        /* Blit the blurred corner to the stencil buffer*/

        /* set the translate matrix*/
        nema_mat3x3_load_identity(m);
        nema_mat3x3_translate(m, -(blend_area.x1 - layer_buf_start_x), -(blend_area.y1 - layer_buf_start_y));
        nema_set_matrix(m);

        /* draw the blurred corner to the stencil buffer*/
        nema_raster_rect(clip_raster_area.x1 - layer_buf_start_x, clip_raster_area.y1 - layer_buf_start_y,
                         clip_raster_area.x2 - clip_raster_area.x1 + 1, clip_raster_area.y2 - clip_raster_area.y1 + 1);
    }



    /*Bottom right corner.*/
    blend_area.x2 = dsc->area.x2;
    blend_area.x1 = dsc->area.x2 - radius + 1;
    blend_area.y1 = dsc->area.y2 - radius + 1;
    blend_area.y2 = dsc->area.y2;
    lv_area_copy(&raster_area, &blend_area);

    if(lv_area_intersect(&clip_raster_area, &raster_area, &t->clip_area)) {

        /* Blit the blurred corner to the stencil buffer*/

        /* set the translate matrix*/
        nema_mat3x3_load_identity(m);
        m[1][1] = -1;
        m[0][2] = blend_area.x1 - layer_buf_start_x;
        m[1][2] = radius - 1 + blend_area.y1 - layer_buf_start_y;
        nema_mat3x3_invert(m);
        nema_set_matrix(m);

        /* draw the blurred corner to the stencil buffer*/
        nema_raster_rect(clip_raster_area.x1 - layer_buf_start_x, clip_raster_area.y1 - layer_buf_start_y,
                         clip_raster_area.x2 - clip_raster_area.x1 + 1, clip_raster_area.y2 - clip_raster_area.y1 + 1);
    }

    /*Bottom left corner.*/
    blend_area.x1 = dsc->area.x1 ;
    blend_area.x2 = dsc->area.x1 + radius - 1;
    blend_area.y1 = dsc->area.y2 - radius + 1;
    blend_area.y2 = dsc->area.y2;
    lv_area_copy(&raster_area, &blend_area);

    if(lv_area_intersect(&clip_raster_area, &raster_area, &t->clip_area)) {
        /* Blit the blurred corner to the stencil buffer*/

        /* set the translate matrix*/
        nema_mat3x3_load_identity(m);
        m[0][0] = -1;
        m[1][1] = -1;
        m[0][2] = radius - 1 + blend_area.x1 - layer_buf_start_x;
        m[1][2] = radius - 1 + blend_area.y1 - layer_buf_start_y;
        nema_mat3x3_invert(m);
        nema_set_matrix(m);

        /* draw the blurred corner to the stencil buffer*/
        nema_raster_rect(clip_raster_area.x1 - layer_buf_start_x, clip_raster_area.y1 - layer_buf_start_y,
                         clip_raster_area.x2 - clip_raster_area.x1 + 1, clip_raster_area.y2 - clip_raster_area.y1 + 1);
    }

    /*Top left corner*/
    blend_area.x1 = dsc->area.x1;
    blend_area.x2 = dsc->area.x1 + radius - 1;
    blend_area.y1 = dsc->area.y1;
    blend_area.y2 = dsc->area.y1 + radius - 1;
    lv_area_copy(&raster_area, &blend_area);

    if(lv_area_intersect(&clip_raster_area, &raster_area, &t->clip_area)) {
        /* set the translate matrix*/
        nema_mat3x3_load_identity(m);
        m[0][0] = -1;
        m[0][2] = radius - 1 + blend_area.x1 - layer_buf_start_x;
        m[1][2] = blend_area.y1 - layer_buf_start_y;
        nema_mat3x3_invert(m);
        nema_set_matrix(m);

        /* draw the mask to the layer buffer*/
        nema_raster_rect(clip_raster_area.x1 - layer_buf_start_x, clip_raster_area.y1 - layer_buf_start_y,
                         clip_raster_area.x2 - clip_raster_area.x1 + 1, clip_raster_area.y2 - clip_raster_area.y1 + 1);
    }


    nema_cmdlist_t * cl = nema_cl_get_bound();
    nema_cl_submit(cl);
    nema_cl_wait(cl);
    nema_cl_rewind(cl);
}


#endif /*LV_USE_DRAW_SW*/
