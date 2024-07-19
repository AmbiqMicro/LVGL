/**
 * @file lv_draw_ambiq_arc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_ambiq.h"
#if LV_USE_DRAW_AMBIQ
#include "../../core/lv_refr.h"
#include "../../misc/lv_assert.h"
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

void lv_draw_ambiq_arc(lv_draw_unit_t * draw_unit, const lv_draw_arc_dsc_t * dsc, const lv_area_t * coords)
{
    if(dsc->opa <= LV_OPA_MIN) return;
    if(dsc->width == 0) return;
    if(dsc->start_angle == dsc->end_angle) return;

    int32_t width = dsc->width;
    if(width > dsc->radius) width = dsc->radius;

    lv_area_t area_out = *coords;
    lv_area_t clipped_area;
    if(!lv_area_intersect(&clipped_area, &area_out, draw_unit->clip_area)) return;

    float start_angle = dsc->start_angle;
    float end_angle = dsc->end_angle;

    // make start_angle in [0, 360]
    start_angle = fmod(start_angle, 360.f);
    if(start_angle < 0)
    {
        start_angle += 360.f;
    }

    // make end_angle in [start_angle, 720]
    end_angle = fmod(end_angle, 720.f);
    if(end_angle < 0)
    {
        end_angle += 720.f;
    }
    if(end_angle <= start_angle)
    {
        end_angle += 360.f;
    }

    lv_layer_t * layer = draw_unit->target_layer;
    uint32_t bg_color    = lv_ambiq_color_convert(dsc->color, dsc->opa);

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


    if(dsc->img_src)
    {
        // //Only set these following two sections as they are used in set_blend_mode.
        // lv_draw_img_dsc_t bg_img_draw_dsc;
        // bg_img_draw_dsc.opa = dsc->opa;
        // bg_img_draw_dsc.recolor = dsc->color;

        // const lv_img_dsc_t* bg_img_dsc =  dsc->img_src;

        // //Set Blending Mode
        // set_blend_mode(bg_img_dsc, &bg_img_draw_dsc);

        // /*Center align*/
        // lv_area_t area;
        // area.x1 = center_x - bg_img_header.w / 2;
        // area.y1 = center_y - bg_img_header.h / 2;
        // area.x2 = area.x1 + bg_img_header.w - 1;
        // area.y2 = area.y1 + bg_img_header.h - 1;

        // uint32_t tex_mode = NEMA_FILTER_PS |(bg_img_header.morton_order ? NEMA_TEX_MORTON_ORDER : 0);

        // if((bg_img_header.cf >= LV_IMG_CF_INDEXED_1BIT) &&
        //     (bg_img_header.cf <= LV_IMG_CF_INDEXED_8BIT))
        // {
        //     uintptr_t palette_addr   = (uintptr_t)bg_img_dsc->data;

        //     uint8_t px_size       = lv_img_cf_get_px_size(bg_img_header.cf);
        //     uint32_t palette_size = 1 << px_size;
        //     uint32_t indices_offset = palette_size * sizeof(lv_color_t);
        //     uintptr_t indices_addr   = palette_addr + indices_offset;
        //     nema_bind_lut_tex(indices_addr, 
        //                       bg_img_header.w, bg_img_header.h,
        //                       bg_img_nema_cf, -1, tex_mode,
        //                       palette_addr, NEMA_RGBA8888);
        // }
        // else
        // {
        //     nema_bind_src_tex((uintptr_t)bg_img_dsc->data,
        //                   bg_img_header.w,
        //                   bg_img_header.h,
        //                   bg_img_nema_cf,
        //                   -1,
        //                   tex_mode);
        // }

        // nema_set_matrix_translate((float)area.x1, (float)area.y1);
    }
    else
    {
        //Set Blending Mode
        nema_set_blend_fill(blending_mode);

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

    return;
    
}

#endif /*LV_USE_DRAW_AMBIQ*/
