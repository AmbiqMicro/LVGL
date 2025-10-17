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
 * @file lv_draw_ambiq_letter.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw_label_private.h"
#include "lv_draw_ambiq.h"
#if LV_USE_DRAW_AMBIQ

#include "lv_draw_ambiq_private.h"
#include "../../display/lv_display.h"
#include "../../misc/lv_math.h"
#include "../../misc/lv_assert.h"
#include "../../misc/lv_area.h"
#include "../../misc/lv_style.h"
#include "../../font/lv_font.h"
#include "../../core/lv_refr_private.h"
#include "../../stdlib/lv_string.h"
#include "../../font/lv_font_fmt_txt.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void /* LV_ATTRIBUTE_FAST_MEM */ draw_letter_cb(lv_draw_task_t * t, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                                       lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *  GLOBAL VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_draw_ambiq_label(lv_draw_task_t * t, const lv_draw_label_dsc_t * dsc, const lv_area_t * coords)
{
    if(dsc->opa <= LV_OPA_MIN) return;

    LV_PROFILER_DRAW_BEGIN;
    lv_draw_label_iterate_characters(t, dsc, coords, draw_letter_cb);
    LV_PROFILER_DRAW_END;
}

void lv_draw_ambiq_letter(lv_draw_task_t * t, const lv_draw_letter_dsc_t * dsc, const lv_area_t * coords)
{
    if(dsc->opa <= LV_OPA_MIN)
        return;

    LV_PROFILER_DRAW_BEGIN;

    lv_draw_glyph_dsc_t glyph_dsc;
    lv_draw_glyph_dsc_init(&glyph_dsc);
    glyph_dsc.opa = dsc->opa;
    glyph_dsc.bg_coords = NULL;
    glyph_dsc.color = dsc->color;
    glyph_dsc.rotation = dsc->rotation;
    glyph_dsc.pivot = dsc->pivot;

    lv_draw_unit_draw_letter(t, &glyph_dsc, &(lv_point_t) {
        .x = coords->x1, .y = coords->y1
    },
    dsc->font, dsc->unicode, draw_letter_cb);

    if(glyph_dsc._draw_buf) {
        lv_draw_buf_destroy(glyph_dsc._draw_buf);
        glyph_dsc._draw_buf = NULL;
    }

    LV_PROFILER_DRAW_END;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/



static void LV_ATTRIBUTE_FAST_MEM draw_letter_cb(lv_draw_task_t * t, lv_draw_glyph_dsc_t * glyph_draw_dsc,
                                                 lv_draw_fill_dsc_t * fill_draw_dsc, const lv_area_t * fill_area)
{

    lv_draw_ambiq_unit_t * draw_ambiq_unit = (lv_draw_ambiq_unit_t *)t->draw_unit;
    lv_layer_t * layer = t->target_layer;

    bool cpu_gpu_sync = false;

    if(fill_draw_dsc == NULL && glyph_draw_dsc == NULL) {
        return;
    }
    if(fill_draw_dsc && glyph_draw_dsc) {
        LV_LOG_WARN("Both fill and glyph draw descriptors are not NULL, use the glyph draw descriptor");
    }


    uint32_t color;
    if(glyph_draw_dsc) {
        color = lv_ambiq_color_convert(glyph_draw_dsc->color, glyph_draw_dsc->opa);
    }
    else {
        color = lv_ambiq_color_convert(fill_draw_dsc->color, fill_draw_dsc->opa);
    }


    lv_area_t raster_coords;

    if(glyph_draw_dsc) {
        switch(glyph_draw_dsc->format) {
            case LV_FONT_GLYPH_FORMAT_NONE: {
#if LV_USE_FONT_PLACEHOLDER
                    if(glyph_draw_dsc->bg_coords == NULL) break;
                    lv_ambiq_set_blend_fill(draw_ambiq_unit, NEMA_BL_SIMPLE);

                    lv_area_copy(&raster_coords, glyph_draw_dsc->bg_coords);
                    lv_area_move(&raster_coords, -layer->buf_area.x1, -layer->buf_area.y1);

                    nema_draw_rect(raster_coords.x1, raster_coords.y1,
                                   lv_area_get_width(&raster_coords), lv_area_get_height(&raster_coords),
                                   color);
#endif
                }
                break;

            case LV_FONT_GLYPH_FORMAT_A1:
            case LV_FONT_GLYPH_FORMAT_A2:
            case LV_FONT_GLYPH_FORMAT_A3:
            case LV_FONT_GLYPH_FORMAT_A4:
            case LV_FONT_GLYPH_FORMAT_A8:

                const lv_font_t * font = glyph_draw_dsc->g->resolved_font;
                lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *)font->dsc;
                lv_font_glyph_dsc_t * g = glyph_draw_dsc->g;

                lv_area_copy(&raster_coords, glyph_draw_dsc->letter_coords);
                lv_area_move(&raster_coords, -layer->buf_area.x1, -layer->buf_area.y1);

                bool is_plain = false;
                if(font->get_glyph_bitmap == lv_font_get_bitmap_fmt_txt) {
                    if(fdsc->bitmap_format == LV_FONT_FMT_TXT_PLAIN) {
                        is_plain = true;
                    }
                }
                if(glyph_draw_dsc->format == LV_FONT_GLYPH_FORMAT_A3) {
                    is_plain = false;
                }

                // set color and blend mode
                if((color & 0xFF000000U) == 0xFF000000U) {
                    lv_ambiq_set_blend_blit(NULL, NEMA_BL_SIMPLE);
                }
                else {
                    lv_ambiq_set_blend_blit(NULL, NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_A);
                    nema_set_const_color(color);
                }
                nema_set_tex_color(color);

                lv_ambiq_draw_bitmap_glyph_t ambiq_draw_bitmap_glyph = {
                    .bitmap_w = g->box_w,
                    .bitmap_h = g->box_h,
                    .raster_start_x = raster_coords.x1,
                    .raster_start_y = raster_coords.y1,
                    .raster_w = raster_coords.x2 - raster_coords.x1 + 1,
                    .raster_h = raster_coords.y2 - raster_coords.y1 + 1,
                    .color = color,
                    .rotate_angle = glyph_draw_dsc->rotation,
                    .pivot_x = glyph_draw_dsc->pivot.x,
                    .pivot_y = glyph_draw_dsc->g->box_h + glyph_draw_dsc->g->ofs_y,
                    .temp_buffer_used = false,
                    .temp_buffer = glyph_draw_dsc->_draw_buf->data,
                    .temp_buffer_w = glyph_draw_dsc->_draw_buf->header.w,
                    .temp_buffer_h = glyph_draw_dsc->_draw_buf->header.h,
                    .temp_buffer_cf = NEMA_A8,
                    .temp_buffer_stride = glyph_draw_dsc->_draw_buf->header.stride,
                };

                if(is_plain) {
                    g->req_raw_bitmap = 1;
                    glyph_draw_dsc->glyph_data = font->get_glyph_bitmap(g, NULL);

                    ambiq_draw_bitmap_glyph.nema_format = lv_ambiq_glyph_format_convert(g->format);
                    ambiq_draw_bitmap_glyph.bitmap = glyph_draw_dsc->glyph_data;
                    ambiq_draw_bitmap_glyph.stride = g->stride;
                    lv_ambiq_draw_bitmap_glyph(&ambiq_draw_bitmap_glyph);

                    if(ambiq_draw_bitmap_glyph.temp_buffer_used) {
                        cpu_gpu_sync = true;
                    }
                }
                else {
                    LV_LOG_WARN("CPU GPU sync required for compressed bitmap, Slow down the performance!");
                    g->req_raw_bitmap = 0;
                    glyph_draw_dsc->glyph_data = lv_font_get_glyph_bitmap(glyph_draw_dsc->g, glyph_draw_dsc->_draw_buf);
                    if(glyph_draw_dsc->glyph_data == NULL) {
                        LV_LOG_WARN("Glyph data is NULL");
                        break;
                    }

                    ambiq_draw_bitmap_glyph.nema_format = NEMA_A8;
                    ambiq_draw_bitmap_glyph.bitmap = (void *)glyph_draw_dsc->_draw_buf->data;
                    ambiq_draw_bitmap_glyph.stride = 0;
                    lv_ambiq_draw_bitmap_glyph(&ambiq_draw_bitmap_glyph);
                    cpu_gpu_sync = true;
                }
                break;


            case LV_FONT_GLYPH_FORMAT_IMAGE: {
#if LV_USE_IMGFONT
                    glyph_draw_dsc->glyph_data = lv_font_get_glyph_bitmap(glyph_draw_dsc->g, glyph_draw_dsc->_draw_buf);

                    lv_draw_image_dsc_t img_dsc;
                    lv_draw_image_dsc_init(&img_dsc);
                    img_dsc.rotation = glyph_draw_dsc->rotation;
                    img_dsc.scale_x = LV_SCALE_NONE;
                    img_dsc.scale_y = LV_SCALE_NONE;
                    img_dsc.opa = glyph_draw_dsc->opa;
                    img_dsc.src = glyph_draw_dsc->glyph_data;
                    img_dsc.recolor = glyph_draw_dsc->color;
                    img_dsc.pivot = (lv_point_t) {
                        .x = glyph_draw_dsc->pivot.x,
                        .y = glyph_draw_dsc->g->box_h + glyph_draw_dsc->g->ofs_y
                    };
                    lv_draw_ambiq_image(t, &img_dsc, glyph_draw_dsc->letter_coords);
#endif
                }
                break;

            case LV_FONT_GLYPH_FORMAT_VECTOR: {
                    lv_draw_ambiq_vector_font(t, glyph_draw_dsc);
                }
                break;


            default:
                break;
        }

    }

    if(fill_draw_dsc && fill_area) {
        lv_ambiq_set_blend_fill(draw_ambiq_unit, NEMA_BL_SIMPLE);

        lv_area_copy(&raster_coords, fill_area);
        lv_area_move(&raster_coords, -layer->buf_area.x1, -layer->buf_area.y1);

        nema_fill_rect(raster_coords.x1, raster_coords.y1,
                       lv_area_get_width(&raster_coords), lv_area_get_height(&raster_coords),
                       color);
    }

    if(cpu_gpu_sync) {
        nema_cmdlist_t * current_cl = nema_cl_get_bound();
        nema_cl_submit(current_cl);
        nema_cl_wait(current_cl);
        nema_cl_rewind(current_cl);
    }
}

#endif /*LV_USE_DRAW_AMBIQ*/
