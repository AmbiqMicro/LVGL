/**
 * @file lv_draw_ambiq_arc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_ambiq.h"
#include "../lv_image_decoder_private.h"
#if LV_USE_DRAW_AMBIQ
#include "../../core/lv_refr.h"
#include "../../misc/lv_assert.h"
#include "lv_draw_ambiq_private.h"
#include <math.h>

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

uint32_t bind_background_image(const lv_draw_arc_dsc_t * dsc, lv_image_decoder_dsc_t* decoder_dsc, uint32_t blending_mode)
{
    const lv_image_header_t* header = &decoder_dsc->decoded->header;
    uint32_t bg_color = lv_ambiq_color_convert(dsc->color, dsc->opa);
    nema_tex_format_t nema_cf = lv_ambiq_color_format_map_src(header->cf);


    if(dsc->opa < LV_OPA_MAX)
    {
        blending_mode |= NEMA_BLOP_MODULATE_A;

        uint32_t global_opa = dsc->opa;
        nema_set_const_color(global_opa<<24);
    }

    // handle look up table(LUT) color format
    if((header->cf == LV_COLOR_FORMAT_I1) ||
    (header->cf == LV_COLOR_FORMAT_I2) ||
    (header->cf == LV_COLOR_FORMAT_I4) ||
    (header->cf == LV_COLOR_FORMAT_I8))
    {
        blending_mode |= NEMA_BLOP_LUT;

        uint32_t lut_size;
        switch(header->cf) {
            case LV_COLOR_FORMAT_I1:
                lut_size = 2U;
                break;
            case LV_COLOR_FORMAT_I2:
                lut_size = 4U;
                break;
            case LV_COLOR_FORMAT_I4:
                lut_size = 16U;
                break;
            default:
                lut_size = 256U;
                break;
        }

        // LUT/PALETTE
        nema_bind_tex(NEMA_TEX2,
                      (uintptr_t)decoder_dsc->palette,
                      lut_size,
                      1,
                      NEMA_BGRA8888,
                      0,
                      NEMA_TEX_REPEAT);
    }

    // handle alpha only color format
    if((header->cf == LV_COLOR_FORMAT_A1) ||
    (header->cf == LV_COLOR_FORMAT_A2) ||
    (header->cf == LV_COLOR_FORMAT_A4) ||
    (header->cf == LV_COLOR_FORMAT_A8))
    {
        nema_set_tex_color(bg_color);
    }
    else
    {
        nema_set_tex_color(0x0);
    }

    // handle mask
    if(header->cf == LV_COLOR_FORMAT_RGB565A8)
    {
        nema_bind_tex(NEMA_TEX3,
                      (uintptr_t)(decoder_dsc->decoded->data + header->h*header->stride),
                      header->w,
                      header->h,
                      NEMA_A8,
                      -1,
                      NEMA_TEX_BORDER);

        blending_mode |= NEMA_BLOP_STENCIL_TXTY;
    }


    //bind image
    uint32_t tex_wrap_mode = NEMA_TEX_BORDER;
    nema_bind_tex(NEMA_TEX1,
                (uintptr_t)decoder_dsc->decoded->data,
                header->w,
                header->h,
                nema_cf,
                header->stride,
                NEMA_FILTER_PS|tex_wrap_mode);
    

    return blending_mode;
}


void lv_draw_ambiq_arc(lv_draw_task_t * t, const lv_draw_arc_dsc_t * dsc, const lv_area_t * coords)
{
    if(dsc->opa <= LV_OPA_MIN) return;
    if(dsc->width == 0) return;
    if(dsc->start_angle == dsc->end_angle) return;

    int32_t width = dsc->width;
    if(width > dsc->radius) width = dsc->radius;

    lv_area_t area_out = *coords;
    lv_area_t clipped_area;
    if(!lv_area_intersect(&clipped_area, &area_out, &t->clip_area)) return;

    float start_angle = dsc->start_angle;
    float end_angle = dsc->end_angle;

    // make start_angle in [0, 360]
    start_angle = fmod(start_angle, 360.f);
    if(start_angle < 0)
    {
        start_angle += 360.f;
    }

    // make end_angle in [start_angle, 720]
    end_angle = fmod(end_angle, 360.f);
    if(end_angle < 0)
    {
        end_angle += 360.f;
    }
    if(end_angle <= start_angle)
    {
        end_angle += 360.f;
    }

    lv_layer_t * layer = t->target_layer;
    uint32_t bg_color    = lv_ambiq_color_convert(dsc->color, dsc->opa);

    bool is_full_circle = false;
    if(end_angle - start_angle > 359.9f && end_angle - start_angle < 360.1f)
    {
        is_full_circle = true;
        nema_set_clip_temp(coords->x1 - layer->buf_area.x1 , coords->y1 - layer->buf_area.y1, lv_area_get_width(coords), lv_area_get_height(coords));
    }

    uint32_t blending_mode;

    if(layer->color_format == LV_COLOR_FORMAT_ARGB8888)
    {
        blending_mode = NEMA_BL_SRC_OVER|NEMA_BLOP_SRC_PREMULT;
    }
    else
    {
        blending_mode = NEMA_BL_SIMPLE;
    }
    
    //Make the coords area relative to the display area
    int32_t center_x = dsc->center.x - layer->buf_area.x1;
    int32_t center_y = dsc->center.y - layer->buf_area.y1;

    lv_image_decoder_dsc_t decoder_dsc;
    bool bind_bg_img = false;

    if(dsc->img_src)
    {
        lv_image_decoder_args_t args;
        args.premultiply = false;
        args.stride_align = false;
        args.use_indexed = true;
        args.no_cache = false;
        args.flush_cache = false;

        lv_result_t res = lv_image_decoder_open(&decoder_dsc, dsc->img_src, &args);
        if(res != LV_RESULT_OK) 
        {
            LV_LOG_ERROR("Failed to open image");
        }
        else
        { 
            if(decoder_dsc.decoded == NULL) 
            {
                /*The whole image is not available, we can't draw it with GPU*/
                LV_LOG_WARN("Ambiq GPU needs to load the whole image to GPU accessible RAM.\n");
                lv_image_decoder_close(&decoder_dsc);
            }
            else
            {
                nema_tex_format_t nema_cf = lv_ambiq_color_format_map_src(decoder_dsc.header.cf);
                if(nema_cf == COLOR_FORMAT_INVALID)
                {
                    LV_LOG_WARN("GPU failed, not supported color format!");
                    lv_image_decoder_close(&decoder_dsc);
                }
                else
                {
                    bind_bg_img = true;
                }
            }
        }
    }

    if(bind_bg_img)
    {
        blending_mode = bind_background_image(dsc, &decoder_dsc, blending_mode);

        //Set Blending Mode
        lv_ambiq_set_blend_blit((lv_draw_ambiq_unit_t*)t->draw_unit, blending_mode);

        /*Center align*/
        lv_area_t area;
        area.x1 = center_x - decoder_dsc.header.w / 2;
        area.y1 = center_y - decoder_dsc.header.h / 2;
        area.x2 = area.x1 + decoder_dsc.header.w - 1;
        area.y2 = area.y1 + decoder_dsc.header.h - 1;

        nema_set_matrix_translate((float)area.x1, (float)area.y1);

    }
    else
    {
        //Set Blending Mode
        lv_ambiq_set_blend_fill((lv_draw_ambiq_unit_t*)t->draw_unit, blending_mode);;

        //Set color
        nema_set_raster_color(bg_color);
    }

    float width_f = width;
    float radius_f = dsc->radius;

    // In NemaSDK, the radius_out= radius+width*0.5, radius_in=radius-width*0.5;
    radius_f -= width_f * 0.5f;

    //Draw the arc
    nema_raster_stroked_arc_aa(center_x, center_y, radius_f, width_f, start_angle, end_angle);

    //Draw rounded ending
    if(dsc->rounded)
    {
        float width_cir = width_f * 0.5f;
        float start_cir_x = center_x + radius_f * nema_cos(start_angle);
        float start_cir_y = center_y + radius_f * nema_sin(start_angle);
        float end_cir_x = center_x + radius_f * nema_cos(end_angle);
        float end_cir_y = center_y + radius_f * nema_sin(end_angle);

        nema_raster_stroked_arc_aa(start_cir_x, start_cir_y, width_cir * 0.5f, width_cir, start_angle + 180.f, start_angle + 360.f);
        nema_raster_stroked_arc_aa(end_cir_x, end_cir_y, width_cir * 0.5f, width_cir, end_angle, end_angle + 180.f);
    }

    if (bind_bg_img)
    {
        nema_cmdlist_t * cl = nema_cl_get_bound();
        nema_cl_submit(cl);
        nema_cl_wait(cl);
        nema_cl_rewind(cl);

        lv_image_decoder_close(&decoder_dsc);
    }

    if(is_full_circle)
    {
        nema_set_clip_pop();
    }

    return;
    
}

#endif /*LV_USE_DRAW_AMBIQ*/
