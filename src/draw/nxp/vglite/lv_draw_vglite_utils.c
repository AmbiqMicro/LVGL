/**
 * @file lv_draw_vglite_utils.c
 *
 */

/**
 * MIT License
 *
 * Copyright 2020-2022 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next paragraph)
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_vglite_utils.h"

#if LV_USE_GPU_NXP_VG_LITE
#include "../../../core/lv_refr.h"
#if BLIT_DBG_AREAS
    #include "lv_draw_vglite_blend.h"
#endif

/*********************
 *      DEFINES
 *********************/

#if LV_COLOR_DEPTH==16
    #define VG_LITE_PX_FMT VG_LITE_RGB565
#elif LV_COLOR_DEPTH==32
    #define VG_LITE_PX_FMT VG_LITE_BGRA8888
#else
    #error Only 16bit and 32bit color depth are supported. Set LV_COLOR_DEPTH to 16 or 32.
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**
 * Clean and invalidate cache.
 */
static inline void invalidate_cache(void);

/**********************
 *  STATIC VARIABLES
 **********************/

static vg_lite_buffer_t dest_vgbuf;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

vg_lite_buffer_t * lv_vglite_get_dest_buf(void)
{
    return &dest_vgbuf;
}

void lv_vglite_init_dest_buf(const lv_color_t * dest_buf, const lv_area_t * dest_area, lv_coord_t dest_stride)
{
    lv_vglite_init_buf(&dest_vgbuf, dest_buf, dest_area, dest_stride);
}

#if BLIT_DBG_AREAS
void lv_vglite_dbg_draw_rectangle(lv_color_t * dest_buf, lv_coord_t dest_width, lv_coord_t dest_height,
                                  lv_area_t * fill_area, lv_color_t color)
{
    lv_area_t a;

    /* top line */
    a.x1 = fill_area->x1;
    a.x2 = fill_area->x2;
    a.y1 = fill_area->y1;
    a.y2 = fill_area->y1;
    lv_gpu_nxp_vglite_fill(dest_buf, dest_width, dest_height, &a, color, LV_OPA_COVER);


    /* bottom line */
    a.x1 = fill_area->x1;
    a.x2 = fill_area->x2;
    a.y1 = fill_area->y2;
    a.y2 = fill_area->y2;
    lv_gpu_nxp_vglite_fill(dest_buf, dest_width, dest_height, &a, color, LV_OPA_COVER);

    /* left line */
    a.x1 = fill_area->x1;
    a.x2 = fill_area->x1;
    a.y1 = fill_area->y1;
    a.y2 = fill_area->y2;
    lv_gpu_nxp_vglite_fill(dest_buf, dest_width, dest_height, &a, color, LV_OPA_COVER);

    /* right line */
    a.x1 = fill_area->x2;
    a.x2 = fill_area->x2;
    a.y1 = fill_area->y1;
    a.y2 = fill_area->y2;
    lv_gpu_nxp_vglite_fill(dest_buf, dest_width, dest_height, &a, color, LV_OPA_COVER);
}
#endif /* BLIT_DBG_AREAS */

lv_res_t lv_vglite_run(void)
{
    invalidate_cache();

    VG_LITE_ERR_RETURN_INV(vg_lite_flush(), "Flush failed.");

    return LV_RES_OK;
}

lv_res_t lv_vglite_premult_and_swizzle(vg_lite_color_t * vg_col32, lv_color32_t lv_col32, lv_opa_t opa,
                                       vg_lite_buffer_format_t vg_col_format)
{

    lv_color32_t lv_col32_premul = lv_col32;
    if(opa <= (lv_opa_t)LV_OPA_MAX) {
        /* Only pre-multiply color if hardware pre-multiplication is not present */
        if(!vg_lite_query_feature(gcFEATURE_BIT_VG_PE_PREMULTIPLY)) {
            lv_col32_premul.ch.red = (uint8_t)(((uint16_t)lv_col32.ch.red * opa) >> 8);
            lv_col32_premul.ch.green = (uint8_t)(((uint16_t)lv_col32.ch.green * opa) >> 8);
            lv_col32_premul.ch.blue = (uint8_t)(((uint16_t)lv_col32.ch.blue * opa) >> 8);
        }
        lv_col32_premul.ch.alpha = opa;
    }

    switch(vg_col_format) {
        case VG_LITE_BGRA8888:
            *vg_col32 = lv_col32_premul.full;
            break;
        case VG_LITE_RGBA8888:
            *vg_col32 = ((uint32_t)lv_col32_premul.ch.red << 24) | ((uint32_t)lv_col32_premul.ch.green << 16) |
                        ((uint32_t)lv_col32_premul.ch.blue << 8) | (uint32_t)lv_col32_premul.ch.alpha;
            break;
        case VG_LITE_ABGR8888:
            *vg_col32 = ((uint32_t)lv_col32_premul.ch.alpha << 24) | ((uint32_t)lv_col32_premul.ch.blue << 16) |
                        ((uint32_t)lv_col32_premul.ch.green << 8) | (uint32_t)lv_col32_premul.ch.red;
            break;
        case VG_LITE_ARGB8888:
            *vg_col32 = ((uint32_t)lv_col32_premul.ch.alpha << 24) | ((uint32_t)lv_col32_premul.ch.red << 16) |
                        ((uint32_t)lv_col32_premul.ch.green << 8) | (uint32_t)lv_col32_premul.ch.blue;
            break;
        default:
            return LV_RES_INV;
    }

    return LV_RES_OK;
}

vg_lite_blend_t lv_vglite_get_blend_mode(lv_blend_mode_t lv_blend_mode)
{
    vg_lite_blend_t vg_blend_mode;
    switch(lv_blend_mode) {
        case LV_BLEND_MODE_ADDITIVE:
            vg_blend_mode = VG_LITE_BLEND_ADDITIVE;
            break;
        case LV_BLEND_MODE_SUBTRACTIVE:
            vg_blend_mode = VG_LITE_BLEND_SUBTRACT;
            break;
        case LV_BLEND_MODE_MULTIPLY:
            vg_blend_mode = VG_LITE_BLEND_MULTIPLY;
            break;
        case LV_BLEND_MODE_REPLACE:
            vg_blend_mode = VG_LITE_BLEND_NONE;
            break;
        default:
            vg_blend_mode = VG_LITE_BLEND_SRC_OVER;
            break;
    }
    return vg_blend_mode;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline void invalidate_cache(void)
{
    lv_disp_t * disp = _lv_refr_get_disp_refreshing();
    if(disp->driver->clean_dcache_cb)
        disp->driver->clean_dcache_cb(disp->driver);
}

void lv_vglite_init_buf(vg_lite_buffer_t * vgbuf, const lv_color_t * buf, const lv_area_t * area,
                        lv_coord_t stride)
{
    vgbuf->format = VG_LITE_PX_FMT;
    vgbuf->tiled = VG_LITE_LINEAR;
    vgbuf->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    vgbuf->transparency_mode = VG_LITE_IMAGE_OPAQUE;

    vgbuf->width = (int32_t)lv_area_get_width(area);
    vgbuf->height = (int32_t)lv_area_get_height(area);
    vgbuf->stride = (int32_t)(stride) * sizeof(lv_color_t);

    lv_memset_00(&vgbuf->yuv, sizeof(vgbuf->yuv));

    vgbuf->memory = (void *)buf;
    vgbuf->address = (uint32_t)vgbuf->memory;
    vgbuf->handle = NULL;
}
#endif /*LV_USE_GPU_NXP_VG_LITE*/
