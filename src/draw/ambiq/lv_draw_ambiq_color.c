/**
 * @file lv_draw_ambiq_color.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_ambiq_color.h"

#if LV_USE_DRAW_AMBIQ

#include "nema_graphics.h"

nema_tex_format_t lv_ambiq_color_format_map_src(lv_color_format_t lvgl_cf)
{
    switch(lvgl_cf)
    {
        case LV_COLOR_FORMAT_L8:
            return NEMA_L8;

        case LV_COLOR_FORMAT_AL88:
            return NEMA_AL88;

        case LV_COLOR_FORMAT_I1:
            return NEMA_L1;
        case LV_COLOR_FORMAT_I2:
            return NEMA_L2;
        case LV_COLOR_FORMAT_I4:
            return NEMA_L4;
        case LV_COLOR_FORMAT_I8:
            return NEMA_L8;

        case LV_COLOR_FORMAT_A1:
            return NEMA_A1;
        case LV_COLOR_FORMAT_A2:
            return NEMA_A2;
        case LV_COLOR_FORMAT_A4:
            return NEMA_A4;
        case LV_COLOR_FORMAT_A8:
            return NEMA_A8;


        case LV_COLOR_FORMAT_RGB565:
            return NEMA_BGR565; 

        case LV_COLOR_FORMAT_RGB565A8:
            return NEMA_BGR565; 

        case LV_COLOR_FORMAT_RGB888:
            return NEMA_BGR24; 

        case LV_COLOR_FORMAT_ARGB8888:
            return NEMA_BGRA8888;

        case LV_COLOR_FORMAT_XRGB8888:
            return NEMA_BGRX8888;

        default:
            return COLOR_FORMAT_INVALID;
    }
}

nema_tex_format_t lv_ambiq_color_format_map_des(lv_color_format_t lvgl_cf)
{
    switch(lvgl_cf)
    {
        case LV_COLOR_FORMAT_RGB565:
            return NEMA_BGR565; 

        case LV_COLOR_FORMAT_RGB888:
            return NEMA_BGR24; 

        case LV_COLOR_FORMAT_ARGB8888:
            return NEMA_BGRA8888;

        case LV_COLOR_FORMAT_XRGB8888:
            return NEMA_BGRX8888;

        case LV_COLOR_FORMAT_L8:
            return NEMA_L8;
        
        case LV_COLOR_FORMAT_AL88:
            return NEMA_AL88;

        case LV_COLOR_FORMAT_A1:
            return NEMA_L1;
        case LV_COLOR_FORMAT_A2:
            return NEMA_L2;
        case LV_COLOR_FORMAT_A4:
            return NEMA_L4;
        case LV_COLOR_FORMAT_A8:
            return NEMA_L8;

        default:
            return COLOR_FORMAT_INVALID;
    }
}

uint32_t lv_ambiq_color_convert(lv_color_t color, lv_opa_t opa)
{
    return nema_rgba(color.red, color.green, color.blue, opa);
}
#endif