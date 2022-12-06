/**
 * @file lv_draw_vglite.c
 *
 */

/**
 * MIT License
 *
 * Copyright 2022 NXP
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

#include "lv_draw_vglite.h"

#if LV_USE_GPU_NXP_VG_LITE
#include <math.h>
#include "lv_draw_vglite_blend.h"
#include "lv_draw_vglite_line.h"
#include "lv_draw_vglite_rect.h"
#include "lv_draw_vglite_arc.h"

#if LV_COLOR_DEPTH != 32
    #include "../../../core/lv_refr.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lv_draw_vglite_wait_for_finish(lv_draw_ctx_t * draw_ctx);

static void lv_draw_vglite_img_decoded(lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * dsc,
                                       const lv_area_t * coords, const uint8_t * map_p, lv_img_cf_t cf);

static void lv_draw_vglite_blend(lv_draw_ctx_t * draw_ctx, const lv_draw_sw_blend_dsc_t * dsc);

static void lv_draw_vglite_line(lv_draw_ctx_t * draw_ctx, const lv_draw_line_dsc_t * dsc, const lv_point_t * point1,
                                const lv_point_t * point2);

static void lv_draw_vglite_rect(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc, const lv_area_t * coords);

static lv_res_t lv_draw_vglite_bg(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc, const lv_area_t * coords);

static lv_res_t lv_draw_vglite_border(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc,
                                      const lv_area_t * coords);

static lv_res_t lv_draw_vglite_outline(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc,
                                       const lv_area_t * coords);

static void lv_draw_vglite_arc(lv_draw_ctx_t * draw_ctx, const lv_draw_arc_dsc_t * dsc, const lv_point_t * center,
                               uint16_t radius, uint16_t start_angle, uint16_t end_angle);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_vglite_ctx_init(lv_disp_drv_t * drv, lv_draw_ctx_t * draw_ctx)
{
    lv_draw_sw_init_ctx(drv, draw_ctx);

    lv_draw_vglite_ctx_t * vglite_draw_ctx = (lv_draw_sw_ctx_t *)draw_ctx;
    vglite_draw_ctx->base_draw.draw_line = lv_draw_vglite_line;
    vglite_draw_ctx->base_draw.draw_arc = lv_draw_vglite_arc;
    vglite_draw_ctx->base_draw.draw_rect = lv_draw_vglite_rect;
    vglite_draw_ctx->base_draw.draw_img_decoded = lv_draw_vglite_img_decoded;
    vglite_draw_ctx->blend = lv_draw_vglite_blend;
    vglite_draw_ctx->base_draw.wait_for_finish = lv_draw_vglite_wait_for_finish;
}

void lv_draw_vglite_ctx_deinit(lv_disp_drv_t * drv, lv_draw_ctx_t * draw_ctx)
{
    lv_draw_sw_deinit_ctx(drv, draw_ctx);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * During rendering, LVGL might initializes new draw_ctxs and start drawing into
 * a separate buffer (called layer). If the content to be rendered has "holes",
 * e.g. rounded corner, LVGL temporarily sets the disp_drv.screen_transp flag.
 * It means the renderers should draw into an ARGB buffer.
 * With 32 bit color depth it's not a big problem but with 16 bit color depth
 * the target pixel format is ARGB8565 which is not supported by the GPU.
 * In this case, the VG-Lite callbacks should fallback to SW rendering.
 */
static inline bool need_argb8565_support()
{
#if LV_COLOR_DEPTH != 32
    lv_disp_t * disp = _lv_refr_get_disp_refreshing();

    if(disp->driver->screen_transp == 1)
        return true;
#endif

    return false;
}

static void lv_draw_vglite_wait_for_finish(lv_draw_ctx_t * draw_ctx)
{
    vg_lite_finish();

    lv_draw_sw_wait_for_finish(draw_ctx);
}

static void lv_draw_vglite_blend(lv_draw_ctx_t * draw_ctx, const lv_draw_sw_blend_dsc_t * dsc)
{
    if(need_argb8565_support()) {
        lv_draw_sw_blend_basic(draw_ctx, dsc);
        return;
    }

    lv_area_t blend_area;
    /*Let's get the blend area which is the intersection of the area to fill and the clip area.*/
    if(!_lv_area_intersect(&blend_area, dsc->blend_area, draw_ctx->clip_area))
        return; /*Fully clipped, nothing to do*/

    /*Make the blend area relative to the buffer*/
    lv_area_move(&blend_area, -draw_ctx->buf_area->x1, -draw_ctx->buf_area->y1);

    bool done = false;
    /*Fill/Blend only non masked, normal blended*/
    if(dsc->mask_buf == NULL && dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        lv_color_t * dest_buf = draw_ctx->buf;
        lv_coord_t dest_stride = lv_area_get_width(draw_ctx->buf_area);
        lv_coord_t dest_width = lv_area_get_width(draw_ctx->buf_area);
        lv_coord_t dest_height = lv_area_get_height(draw_ctx->buf_area);

        const lv_color_t * src_buf = dsc->src_buf;

        if(src_buf == NULL) {
            done = (lv_gpu_nxp_vglite_fill(dest_buf, dest_width, dest_height, &blend_area,
                                           dsc->color, dsc->opa) == LV_RES_OK);
            if(!done)
                VG_LITE_LOG_TRACE("VG-Lite fill failed. Fallback.");
        }
        else {
            lv_area_t src_area;
            lv_coord_t src_width = lv_area_get_width(dsc->blend_area);
            lv_coord_t src_height = lv_area_get_height(dsc->blend_area);
            src_area.x1 = blend_area.x1 - (dsc->blend_area->x1 - draw_ctx->buf_area->x1);
            src_area.y1 = blend_area.y1 - (dsc->blend_area->y1 - draw_ctx->buf_area->y1);
            src_area.x2 = src_area.x1 + src_width - 1;
            src_area.y2 = src_area.y1 + src_height - 1;

            lv_gpu_nxp_vglite_blit_info_t blit;
            lv_coord_t src_stride = lv_area_get_width(dsc->blend_area);

            blit.src = src_buf;
            blit.src_width = src_width;
            blit.src_height = src_height;
            blit.src_stride = src_stride;
            blit.src_area = src_area;

            blit.dst = dest_buf;
            blit.dst_width = dest_width;
            blit.dst_height = dest_height;
            blit.dst_stride = dest_stride;
            blit.dst_area.x1 = blend_area.x1;
            blit.dst_area.y1 = blend_area.y1;
            blit.dst_area.x2 = blend_area.x2;
            blit.dst_area.y2 = blend_area.y2;

            blit.opa = dsc->opa;
            blit.zoom = LV_IMG_ZOOM_NONE;
            blit.angle = 0;

            done = (lv_gpu_nxp_vglite_blit(&blit) == LV_RES_OK);

            if(!done)
                VG_LITE_LOG_TRACE("VG-Lite blit failed. Fallback.");
        }
    }

    if(!done)
        lv_draw_sw_blend_basic(draw_ctx, dsc);
}

static void lv_draw_vglite_img_decoded(lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * dsc,
                                       const lv_area_t * coords, const uint8_t * map_p, lv_img_cf_t cf)
{
    if(need_argb8565_support()) {
        lv_draw_sw_img_decoded(draw_ctx, dsc, coords, map_p, cf);
        return;
    }

    /*Use the clip area as draw area*/
    lv_area_t draw_area;
    lv_area_copy(&draw_area, draw_ctx->clip_area);
    bool mask_any = lv_draw_mask_is_any(&draw_area);
    bool recolor = (dsc->recolor_opa != LV_OPA_TRANSP);

    lv_area_t blend_area;
    /*Let's get the blend area which is the intersection of the area to fill and the clip area.*/
    if(!_lv_area_intersect(&blend_area, coords, draw_ctx->clip_area))
        return; /*Fully clipped, nothing to do*/

    /*Make the blend area relative to the buffer*/
    lv_area_move(&blend_area, -draw_ctx->buf_area->x1, -draw_ctx->buf_area->y1);

    const lv_color_t * src_buf = (const lv_color_t *)map_p;
    if(!src_buf) {
        lv_draw_sw_img_decoded(draw_ctx, dsc, coords, map_p, cf);
        return;
    }

    lv_color_t * dest_buf = draw_ctx->buf;
    lv_coord_t dest_stride = lv_area_get_width(draw_ctx->buf_area);

    lv_area_t src_area;
    lv_coord_t src_width = lv_area_get_width(coords);
    lv_coord_t src_height = lv_area_get_height(coords);
    src_area.x1 = blend_area.x1 - (coords->x1 - draw_ctx->buf_area->x1);
    src_area.y1 = blend_area.y1 - (coords->y1 - draw_ctx->buf_area->y1);
    src_area.x2 = src_area.x1 + src_width - 1;
    src_area.y2 = src_area.y1 + src_height - 1;

    bool done = false;

    if(!mask_any &&
       !lv_img_cf_is_chroma_keyed(cf) && !recolor
#if LV_COLOR_DEPTH!=32
       && !lv_img_cf_has_alpha(cf)
#endif
      ) {
        lv_gpu_nxp_vglite_blit_info_t blit;
        lv_coord_t src_stride = lv_area_get_width(coords);

        blit.src = src_buf;
        blit.src_width = src_width;
        blit.src_height = src_height;
        blit.src_stride = src_stride;
        blit.src_area = src_area;

        blit.dst = dest_buf;
        blit.dst_width = lv_area_get_width(draw_ctx->buf_area);
        blit.dst_height = lv_area_get_height(draw_ctx->buf_area);
        blit.dst_stride = dest_stride;
        blit.dst_area.x1 = blend_area.x1;
        blit.dst_area.y1 = blend_area.y1;
        blit.dst_area.x2 = blend_area.x2;
        blit.dst_area.y2 = blend_area.y2;

        blit.opa = dsc->opa;
        blit.angle = dsc->angle;
        blit.pivot = dsc->pivot;
        blit.zoom = dsc->zoom;

        done = (lv_gpu_nxp_vglite_blit_transform(&blit) == LV_RES_OK);

        if(!done)
            VG_LITE_LOG_TRACE("VG-Lite blit transform failed. Fallback.");
    }

    if(!done)
        lv_draw_sw_img_decoded(draw_ctx, dsc, coords, map_p, cf);
}

static void lv_draw_vglite_line(lv_draw_ctx_t * draw_ctx, const lv_draw_line_dsc_t * dsc, const lv_point_t * point1,
                                const lv_point_t * point2)
{
    if(need_argb8565_support()) {
        lv_draw_sw_line(draw_ctx, dsc, point1, point2);
        return;
    }

    if(dsc->width == 0)
        return;
    if(dsc->opa <= (lv_opa_t)LV_OPA_MIN)
        return;
    if(point1->x == point2->x && point1->y == point2->y)
        return;

    lv_area_t clip_line;
    clip_line.x1 = LV_MIN(point1->x, point2->x) - dsc->width / 2;
    clip_line.x2 = LV_MAX(point1->x, point2->x) + dsc->width / 2;
    clip_line.y1 = LV_MIN(point1->y, point2->y) - dsc->width / 2;
    clip_line.y2 = LV_MAX(point1->y, point2->y) + dsc->width / 2;

    bool is_common;
    is_common = _lv_area_intersect(&clip_line, &clip_line, draw_ctx->clip_area);
    if(!is_common)
        return;

    bool done = false;
    bool mask_any = lv_draw_mask_is_any(&clip_line);

    if(!mask_any) {
        done = (lv_gpu_nxp_vglite_draw_line(draw_ctx, dsc, point1, point2, &clip_line) == LV_RES_OK);
        if(!done)
            VG_LITE_LOG_TRACE("VG-Lite draw line failed. Fallback.");
    }

    if(!done)
        lv_draw_sw_line(draw_ctx, dsc, point1, point2);
}

static void lv_draw_vglite_rect(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc, const lv_area_t * coords)
{
    if(need_argb8565_support()) {
        lv_draw_sw_rect(draw_ctx, dsc, coords);
        return;
    }

    lv_draw_rect_dsc_t vglite_dsc;

    lv_memcpy(&vglite_dsc, dsc, sizeof(vglite_dsc));
    vglite_dsc.bg_opa = 0;
    vglite_dsc.bg_img_opa = 0;
    vglite_dsc.border_opa = 0;
    vglite_dsc.outline_opa = 0;
#if LV_DRAW_COMPLEX
    /* Draw the shadow with CPU */
    lv_draw_sw_rect(draw_ctx, &vglite_dsc, coords);
    vglite_dsc.shadow_opa = 0;
#endif /*LV_DRAW_COMPLEX*/

    /* Draw the background */
    vglite_dsc.bg_opa = dsc->bg_opa;
    if(lv_draw_vglite_bg(draw_ctx, &vglite_dsc, coords) != LV_RES_OK)
        lv_draw_sw_rect(draw_ctx, &vglite_dsc, coords);
    vglite_dsc.bg_opa = 0;

    /* Draw the background image
     * It will be done once draw_ctx->draw_img_decoded()
     * callback gets called from lv_draw_sw_rect().
     */
    vglite_dsc.bg_img_opa = dsc->bg_img_opa;
    lv_draw_sw_rect(draw_ctx, &vglite_dsc, coords);
    vglite_dsc.bg_img_opa = 0;

    /* Draw the border */
    vglite_dsc.border_opa = dsc->border_opa;
    if(lv_draw_vglite_border(draw_ctx, &vglite_dsc, coords) != LV_RES_OK)
        lv_draw_sw_rect(draw_ctx, &vglite_dsc, coords);
    vglite_dsc.border_opa = 0;

    /* Draw the outline */
    vglite_dsc.outline_opa = dsc->outline_opa;
    if(lv_draw_vglite_outline(draw_ctx, &vglite_dsc, coords) != LV_RES_OK)
        lv_draw_sw_rect(draw_ctx, &vglite_dsc, coords);
}

static lv_res_t lv_draw_vglite_bg(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc, const lv_area_t * coords)
{
    if(dsc->bg_opa <= (lv_opa_t)LV_OPA_MIN)
        return LV_RES_INV;

    lv_area_t bg_coords;
    lv_area_copy(&bg_coords, coords);

    /*If the border fully covers make the bg area 1px smaller to avoid artifacts on the corners*/
    if(dsc->border_width > 1 && dsc->border_opa >= (lv_opa_t)LV_OPA_MAX && dsc->radius != 0) {
        bg_coords.x1 += (dsc->border_side & LV_BORDER_SIDE_LEFT) ? 1 : 0;
        bg_coords.y1 += (dsc->border_side & LV_BORDER_SIDE_TOP) ? 1 : 0;
        bg_coords.x2 -= (dsc->border_side & LV_BORDER_SIDE_RIGHT) ? 1 : 0;
        bg_coords.y2 -= (dsc->border_side & LV_BORDER_SIDE_BOTTOM) ? 1 : 0;
    }

    lv_area_t clipped_coords;
    if(!_lv_area_intersect(&clipped_coords, &bg_coords, draw_ctx->clip_area))
        return LV_RES_INV;

    bool mask_any = lv_draw_mask_is_any(&bg_coords);
    lv_grad_dir_t grad_dir = dsc->bg_grad.dir;
    lv_color_t bg_color = (grad_dir == (lv_grad_dir_t)LV_GRAD_DIR_NONE) ?
                          dsc->bg_color : dsc->bg_grad.stops[0].color;
    if(bg_color.full == dsc->bg_grad.stops[1].color.full)
        grad_dir = LV_GRAD_DIR_NONE;

    /*
     * Most simple case: just a plain rectangle (no mask, no radius, no gradient)
     * shall be handled by draw_ctx->blend().
     *
     * Complex case: gradient or radius but no mask.
     */
    if(!mask_any && ((dsc->radius != 0) || (grad_dir != (lv_grad_dir_t)LV_GRAD_DIR_NONE))) {
        lv_res_t res = lv_gpu_nxp_vglite_draw_bg(draw_ctx, dsc, &bg_coords);
        if(res != LV_RES_OK)
            VG_LITE_LOG_TRACE("VG-Lite draw bg failed. Fallback.");

        return res;
    }

    return LV_RES_INV;
}

static lv_res_t lv_draw_vglite_border(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc,
                                      const lv_area_t * coords)
{
    if(dsc->border_opa <= (lv_opa_t)LV_OPA_MIN)
        return LV_RES_INV;
    if(dsc->border_width == 0)
        return LV_RES_INV;
    if(dsc->border_post)
        return LV_RES_INV;
    if(dsc->border_side != (lv_border_side_t)LV_BORDER_SIDE_FULL)
        return LV_RES_INV;

    lv_area_t border_coords;
    lv_coord_t border_width = dsc->border_width;

    /* Move border inwards to align with software rendered border */
    border_coords.x1 = coords->x1 + ceil(border_width / 2.0f);
    border_coords.x2 = coords->x2 - floor(border_width / 2.0f);
    border_coords.y1 = coords->y1 + ceil(border_width / 2.0f);
    border_coords.y2 = coords->y2 - floor(border_width / 2.0f);

    lv_res_t res = lv_gpu_nxp_vglite_draw_border_generic(draw_ctx, dsc, &border_coords, true);
    if(res != LV_RES_OK)
        VG_LITE_LOG_TRACE("VG-Lite draw border failed. Fallback.");

    return res;
}

static lv_res_t lv_draw_vglite_outline(lv_draw_ctx_t * draw_ctx, const lv_draw_rect_dsc_t * dsc,
                                       const lv_area_t * coords)
{
    if(dsc->outline_opa <= (lv_opa_t)LV_OPA_MIN)
        return LV_RES_INV;
    if(dsc->outline_width == 0)
        return LV_RES_INV;

    /* Move outline outwards to align with software rendered outline */
    lv_coord_t outline_pad = dsc->outline_pad - 1;
    lv_area_t outline_coords;
    outline_coords.x1 = coords->x1 - outline_pad - floor(dsc->outline_width / 2.0f);
    outline_coords.x2 = coords->x2 + outline_pad + ceil(dsc->outline_width / 2.0f);
    outline_coords.y1 = coords->y1 - outline_pad - floor(dsc->outline_width / 2.0f);
    outline_coords.y2 = coords->y2 + outline_pad + ceil(dsc->outline_width / 2.0f);

    lv_res_t res = lv_gpu_nxp_vglite_draw_border_generic(draw_ctx, dsc, &outline_coords, false);
    if(res != LV_RES_OK)
        VG_LITE_LOG_TRACE("VG-Lite draw outline failed. Fallback.");

    return res;
}

static void lv_draw_vglite_arc(lv_draw_ctx_t * draw_ctx, const lv_draw_arc_dsc_t * dsc, const lv_point_t * center,
                               uint16_t radius, uint16_t start_angle, uint16_t end_angle)
{
    if(need_argb8565_support()) {
        lv_draw_sw_arc(draw_ctx, dsc, center, radius, start_angle, end_angle);
        return;
    }

    bool done = false;

#if LV_DRAW_COMPLEX
    if(dsc->opa <= (lv_opa_t)LV_OPA_MIN)
        return;
    if(dsc->width == 0)
        return;
    if(start_angle == end_angle)
        return;

    done = (lv_gpu_nxp_vglite_draw_arc(draw_ctx, dsc, center, (int32_t)radius,
                                       (int32_t)start_angle, (int32_t)end_angle) == LV_RES_OK);
    if(!done)
        VG_LITE_LOG_TRACE("VG-Lite draw arc failed. Fallback.");
#endif/*LV_DRAW_COMPLEX*/

    if(!done)
        lv_draw_sw_arc(draw_ctx, dsc, center, radius, start_angle, end_angle);
}

#endif /*LV_USE_GPU_NXP_VG_LITE*/
