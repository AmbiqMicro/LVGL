/**
 * @file lv_draw_ambiq_vector.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "../lv_image_decoder_private.h"
#include "../lv_draw_vector_private.h"
#include "lv_draw_ambiq.h"

#if LV_USE_DRAW_AMBIQ && LV_USE_VECTOR_GRAPHIC

#if LV_USE_DRAW_AMBIQ_VG==0
#error "LV_USE_VECTOR_GRAPHIC requires LV_USE_DRAW_AMBIQ_VG 1"
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

static void task_draw_cb(void * ctx, const lv_vector_path_t * path, const lv_vector_draw_dsc_t * dsc);

static void lv_path_to_vg(NEMA_VG_PATH_HANDLE * dest, const lv_vector_path_t * src);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_ambiq_vector(lv_draw_unit_t * draw_unit, const lv_draw_vector_task_dsc_t * dsc)
{
    if(dsc->task_list == NULL)
        return;

    lv_layer_t * layer = dsc->base.layer;
    if(layer->draw_buf == NULL)
        return;

    /*handle the layer buffer offset by setting a global matrix*/
    nema_vg_reset_global_matrix();

    nema_matrix3x3_t matrix;

    nema_mat3x3_load_identity(&matrix);

    int32_t layer_start_x = layer->buf_area.x1;
    int32_t layer_start_y = layer->buf_area.y1;

    nema_mat3x3_translate(&matrix, -layer_start_x, -layer_start_y);

    nema_vg_set_global_matrix(&matrix);

    /*handle each path*/

    LV_PROFILER_BEGIN;
    lv_vector_for_each_destroy_tasks(dsc->task_list, task_draw_cb, draw_unit);
    LV_PROFILER_END;
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

uint32_t lv_vector_blend_to_nema(lv_vector_blend_t blend)
{
    switch(blend) {
        case LV_VECTOR_BLEND_SRC_OVER:
            return NEMA_BL_SRC_OVER;
        case LV_VECTOR_BLEND_SRC_IN:
            return NEMA_BL_SRC_IN;
        case LV_VECTOR_BLEND_DST_OVER:
            return NEMA_BL_DST_OVER;
        case LV_VECTOR_BLEND_DST_IN:
            return NEMA_BL_DST_IN;
        default:
            return 0xffffffff;
    }
}

static uint32_t lv_vector_fill_rule_to_nema(lv_vector_fill_t fill_rule)
{
    switch(fill_rule) {
        case LV_VECTOR_FILL_NONZERO:
            return NEMA_VG_FILL_NON_ZERO;
        case LV_VECTOR_FILL_EVENODD:
            return NEMA_VG_FILL_EVEN_ODD;
        default:
            return NEMA_VG_FILL_NON_ZERO;
    }
}

static uint32_t lv_vector_quality_to_nema(lv_vector_path_quality_t quality)
{
    switch(quality) {
        case LV_VECTOR_PATH_QUALITY_LOW:
            return NEMA_VG_QUALITY_NON_AA;
        case LV_VECTOR_PATH_QUALITY_MEDIUM:
            return NEMA_VG_QUALITY_FASTER;
        case LV_VECTOR_PATH_QUALITY_HIGH:
            return NEMA_VG_QUALITY_BETTER;
        default:
            return NEMA_VG_QUALITY_FASTER;
    }
}

static void lv_vector_path_to_nema(lv_array_t * dest, const lv_array_t * src)
{
    lv_array_deinit(dest);
    lv_array_init(dest, lv_array_size(src), sizeof(uint8_t));


    lv_vector_path_op_t * op = lv_array_front(src);
    uint8_t* dest_data = lv_array_front(dest);

    uint32_t size = lv_array_size(src);
    for(uint32_t i = 0; i < size; i++) {
        switch(op[i]) {
            case LV_VECTOR_PATH_OP_MOVE_TO: 
                dest_data[i] = NEMA_VG_PRIM_MOVE;
                break;
            case LV_VECTOR_PATH_OP_LINE_TO:
                dest_data[i] = NEMA_VG_PRIM_LINE;
                break;
            case LV_VECTOR_PATH_OP_QUAD_TO:
                dest_data[i] = NEMA_VG_PRIM_BEZIER_QUAD;
                break;
            case LV_VECTOR_PATH_OP_CUBIC_TO:
                dest_data[i] = NEMA_VG_PRIM_BEZIER_CUBIC;
                break;
            case LV_VECTOR_PATH_OP_CLOSE: 
                dest_data[i] = NEMA_VG_PRIM_CLOSE;
                break;
        }
    }
}

static void lv_vector_grad_color_to_nema(NEMA_VG_GRAD_HANDLE  vg_grad, const lv_vector_gradient_t * grad)
{
    float stops[grad->stops_count];
    color_var_t stops_colors[grad->stops_count];

    for(uint32_t i = 0; i < grad->stops_count; i++) {
        stops[i] = (float)grad->stops[i].frac * (1.f/255.f);
        stops_colors[i].a = grad->stops[i].opa;
        stops_colors[i].r = grad->stops[i].color.red;
        stops_colors[i].g = grad->stops[i].color.green;
        stops_colors[i].b = grad->stops[i].color.blue;
    }
    

    nema_vg_grad_clear(vg_grad);
    nema_vg_grad_set(vg_grad, grad->stops_count, stops, stops_colors);
}

static nema_tex_mode_t lv_vector_grad_spread_to_nema(lv_vector_gradient_spread_t spread)
{
    switch(spread) {
        case LV_VECTOR_GRADIENT_SPREAD_PAD:
            return NEMA_TEX_CLAMP;
        case LV_VECTOR_GRADIENT_SPREAD_REFLECT:
            return NEMA_TEX_MIRROR;
        case LV_VECTOR_GRADIENT_SPREAD_REPEAT:
            return NEMA_TEX_REPEAT;
        default:
            return NEMA_TEX_CLAMP;
    }
}

uint32_t bind_background_image(const lv_draw_image_dsc_t * img_dsc, lv_image_decoder_dsc_t* decoder_dsc)
{
    lv_image_header_t* header = &decoder_dsc->decoded->header;
    uint32_t bg_color = lv_ambiq_color_convert(img_dsc->recolor, img_dsc->recolor_opa);
    nema_tex_format_t nema_cf = lv_ambiq_color_format_map_src(header->cf);


    // handle look up table(LUT) color format
    if((header->cf == LV_COLOR_FORMAT_I1) ||
    (header->cf == LV_COLOR_FORMAT_I2) ||
    (header->cf == LV_COLOR_FORMAT_I4) ||
    (header->cf == LV_COLOR_FORMAT_I8))
    {

        nema_vg_bind

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


    //bind image
    uint32_t tex_wrap_mode = NEMA_TEX_BORDER;
    nema_bind_tex(NEMA_TEX1,
                (uintptr_t)decoder_dsc->decoded->data,
                header->w,
                header->h,
                nema_cf,
                -1,
                NEMA_FILTER_PS|tex_wrap_mode);
    

}

static void lv_vector_image_to_nema(NEMA_VG_PAINT_HANDLE vg_paint,
                                    const lv_draw_image_dsc_t * img_dsc)
{

    lv_image_decoder_dsc_t decoder_dsc;


        lv_result_t res = lv_image_decoder_open(&decoder_dsc, img_dsc->src, NULL);
        if(res != LV_RESULT_OK) 
        {
            LV_LOG_ERROR("Failed to open image");
            return;
        }
        else
        { 
            if(decoder_dsc.decoded == NULL) 
            {
                /*The whole image is not available, we can't draw it with GPU*/
                LV_LOG_WARN("Ambiq GPU needs to load the whole image to GPU accessible RAM.\n");
                lv_image_decoder_close(&decoder_dsc);
                return;
            }
            else
            {
                nema_tex_format_t nema_cf = lv_ambiq_color_format_map_src(decoder_dsc.header.cf);
                if(nema_cf == COLOR_FORMAT_INVALID)
                {
                    LV_LOG_WARN("GPU failed, not supported color format!");
                    lv_image_decoder_close(&decoder_dsc);
                    return ;
                }

                if(decoder_dsc.header.cf == LV_COLOR_FORMAT_RGB565A8)
                {
                    LV_LOG_WARN("not support RGB565A8 color format for vector graphics!");
                    lv_image_decoder_close(&decoder_dsc);
                    return ;   
                }

            }
        }
    
    lv_image_header_t* header = &decoder_dsc.decoded->header;
    uint32_t bg_color = lv_ambiq_color_convert(img_dsc->recolor, img_dsc->recolor_opa);
    nema_tex_format_t nema_cf = lv_ambiq_color_format_map_src(header->cf);
    bool LUT_texture = false;

    nema_image_obj_t* img_obj = lv_malloc(sizeof(nema_image_obj_t));
    nema_image_obj_t* palette_obj = lv_malloc(sizeof(nema_image_obj_t));

    //bind image
    img_obj->bo.base_virt = (void *)decoder_dsc.decoded->data;
    img_obj->bo.base_phys = (uintptr_t)decoder_dsc.decoded->data;
    img_obj->bo.size = decoder_dsc.decoded->data_size;
    img_obj->format = nema_cf;
    img_obj->w = header->w;
    img_obj->h = header->h;
    img_obj->stride = -1;
    img_obj->sampling_mode = NEMA_FILTER_BL|NEMA_TEX_BORDER;

    // handle look up table(LUT) color format
    if((header->cf == LV_COLOR_FORMAT_I1) ||
    (header->cf == LV_COLOR_FORMAT_I2) ||
    (header->cf == LV_COLOR_FORMAT_I4) ||
    (header->cf == LV_COLOR_FORMAT_I8))
    {

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

        // LUT PALETTE
        palette_obj->bo.base_virt = (void *)decoder_dsc.palette;
        palette_obj->bo.base_phys = (uintptr_t)decoder_dsc.palette;
        palette_obj->bo.size = lut_size*4;
        palette_obj->format = NEMA_BGRA8888;
        palette_obj->w = lut_size;
        palette_obj->h = 1;
        palette_obj->stride = -1;

        LUT_texture = true;
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

    if(LUT_texture)
    {
        nema_vg_paint_set_lut_tex(vg_paint, palette_obj, img_obj);
    }
    else
    {
        nema_vg_paint_set_tex(vg_paint, img_obj);
    }
    


}

static void lv_vector_paint_to_nema(NEMA_VG_PAINT_HANDLE vg_paint, NEMA_VG_GRAD_HANDLE vg_grad, const lv_vector_fill_dsc_t * dsc)
{
    /*clear paint*/
    nema_vg_paint_clear(vg_paint);

    /* clear gradient*/
    nema_vg_gradient_clear(vg_grad);

    /* set fill rule*/
    uint32_t file_rule = lv_vector_fill_rule_to_nema(dsc->fill_rule);
    nema_vg_set_fill_rule(file_rule);

    /*set paint opacity*/
    nema_vg_paint_set_opacity(vg_paint, ((float)dsc->opa)/255.f);

    if(dsc->style == LV_VECTOR_DRAW_STYLE_SOLID)
    {
        nema_vg_paint_set_type(vg_paint, NEMA_VG_PAINT_COLOR);
        nema_vg_paint_color(vg_paint, nema_rgba(dsc->color.red, dsc->color.green, dsc->color.blue, dsc->opa));
    }
    else if(dsc->style == LV_VECTOR_DRAW_STYLE_GRADIENT)
    {    
        lv_vector_grad_color_to_nema(vg_grad, &dsc->gradient);
        nema_tex_mode_t sampling_mode = lv_vector_grad_spread_to_nema(dsc->gradient.spread);

        if(dsc->gradient.style == LV_VECTOR_GRADIENT_STYLE_LINEAR)
        {
            lv_fpoint_t p1;
            lv_fpoint_t p2;

            p1.x = dsc->gradient.x1;
            p1.y = dsc->gradient.y1;
            p2.x = dsc->gradient.x2;
            p2.y = dsc->gradient.y2;

            lv_matrix_transform_point(&dsc->matrix, &p1);
            lv_matrix_transform_point(&dsc->matrix, &p2);

            nema_vg_paint_set_type(vg_paint, NEMA_VG_PAINT_GRAD_LINEAR);
            nema_vg_paint_set_grad_linear(vg_paint, vg_grad, p1.x, p1.y, p2.x, p2.y, sampling_mode);
        }
        else if(dsc->gradient.style == LV_VECTOR_GRADIENT_STYLE_RADIAL)
        {

            lv_fpoint_t p3;
            float s;
            float new_r;

            p3.x = dsc->gradient.cx;
            p3.y = dsc->gradient.cy;
            s = dsc->matrix[0][0] * dsc->matrix[1][1] - dsc->matrix[0][1] * dsc->matrix[1][0];
            LV_ASSERT_MSG(s <= 0, "matrix is not invertible");
            s = sqrt(s);
            new_r = dsc->gradient.cr * s;

            lv_matrix_transform_point(&dsc->matrix, &p3);

            nema_vg_paint_set_type(vg_paint, NEMA_VG_PAINT_GRAD_RADIAL);
            nema_vg_paint_set_grad_radial(vg_paint, vg_grad, p3.x, p3.y,
                                            new_r, sampling_mode);
        }
        else
        {
            LV_LOG_WARN("unsupported gradient style: %d", dsc->gradient.style);
        }
    
    }
    else if(dsc->style == LV_VECTOR_DRAW_STYLE_PATTERN)
    {
        // TODO: support pattern
        nema_vg_paint_set_type(vg_paint, NEMA_VG_PAINT_TEXTURE);

        lv_vector_image_to_nema(vg_paint, &dsc->img_dsc);
        nema_vg_paint_set_tex_matrix(vg_paint, dsc->matrix->m);
    }
    else
    {
        LV_LOG_WARN("unsupported style: %d", dsc->style);
    }
}

static uint8_t lv_vector_stroke_cap_to_nema(lv_vector_stroke_cap_t cap)
{
    switch(cap) {
        case LV_VECTOR_STROKE_CAP_BUTT:
            return NEMA_VG_CAP_BUTT;
        case LV_VECTOR_STROKE_CAP_ROUND:
            return NEMA_VG_CAP_ROUND;
        case LV_VECTOR_STROKE_CAP_SQUARE:
            return NEMA_VG_CAP_SQUARE;
        default:
            return NEMA_VG_CAP_BUTT;
    }
}

static uint8_t lv_vector_stroke_join_to_nema(lv_vector_stroke_join_t join)
{
    switch(join) {
        case LV_VECTOR_STROKE_JOIN_MITER:
            return NEMA_VG_JOIN_MITER;
        case LV_VECTOR_STROKE_JOIN_ROUND:
            return NEMA_VG_JOIN_ROUND;
        case LV_VECTOR_STROKE_JOIN_BEVEL:
            return NEMA_VG_JOIN_BEVEL;
        default:
            return NEMA_VG_JOIN_MITER;
    }
}

static void lv_vector_stroke_to_nema(NEMA_VG_PAINT_HANDLE vg_paint, NEMA_VG_GRAD_HANDLE vg_grad, const lv_vector_stroke_dsc_t * dsc)
{
    /*clear paint*/
    nema_vg_paint_clear(vg_paint);

    /* clear gradient*/
    nema_vg_gradient_clear(vg_grad);

    /* set fill rule*/
    nema_vg_set_fill_rule(NEMA_VG_STROKE);

    /*set paint opacity*/
    nema_vg_paint_set_opacity(vg_paint, ((float)dsc->opa)/255.f);

    /*set stroke width*/
    nema_vg_stroke_set_width(dsc->width);

    /*set stroke cap*/
    nema_vg_stroke_set_cap_style(lv_vector_stroke_cap_to_nema(dsc->cap));

    /*set stroke join*/
    nema_vg_stroke_set_join_style(lv_vector_stroke_join_to_nema(dsc->join));

    /*set stroke miter limit*/
    nema_vg_stroke_set_miter_limit((float)dsc->miter_limit);

    /*do not support dash*/
    if(dsc->dash_pattern.size > 0) {
        LV_LOG_WARN("dash pattern is not supported");
    }

    if(dsc->style == LV_VECTOR_DRAW_STYLE_SOLID)
    {
        nema_vg_paint_set_type(vg_paint, NEMA_VG_PAINT_COLOR);
        nema_vg_paint_color(vg_paint, nema_rgba(dsc->color.red, dsc->color.green, dsc->color.blue, dsc->opa));
    }
    else if(dsc->style == LV_VECTOR_DRAW_STYLE_GRADIENT)
    {    
        lv_vector_grad_color_to_nema(vg_grad, &dsc->gradient);
        nema_tex_mode_t sampling_mode = lv_vector_grad_spread_to_nema(dsc->gradient.spread);

        if(dsc->gradient.style == LV_VECTOR_GRADIENT_STYLE_LINEAR)
        {
            nema_vg_paint_set_type(vg_paint, NEMA_VG_PAINT_GRAD_LINEAR);



            nema_vg_paint_set_grad_linear(vg_paint, vg_grad, dsc->gradient.x1, dsc->gradient.y1,
                                            dsc->gradient.x2, dsc->gradient.y2,
                                            sampling_mode);
        }
        else if(dsc->gradient.style == LV_VECTOR_GRADIENT_STYLE_RADIAL)
        {
            nema_vg_paint_set_type(vg_paint, NEMA_VG_PAINT_GRAD_RADIAL);
            nema_vg_paint_set_grad_radial(vg_paint, vg_grad, dsc->gradient.cx, dsc->gradient.cy,
                                            dsc->gradient.cr, sampling_mode);
        }
        else
        {
            LV_LOG_WARN("unsupported gradient style: %d", dsc->gradient.style);
        }
    
    }
    else
    {
        LV_LOG_WARN("unsupported style: %d", dsc->style);
    }
}

static void task_draw_cb(void * ctx, const lv_vector_path_t * path, const lv_vector_draw_dsc_t * dsc)
{
    LV_PROFILER_BEGIN;
    lv_draw_ambiq_unit_t *unit = (lv_draw_ambiq_unit_t *)ctx;

    /*set clip*/
    lv_layer_t * layer = dsc->base.layer;
    int32_t layer_start_x = layer->buf_area.x1;
    int32_t layer_start_y = layer->buf_area.y1;   

    lv_area_t scissor_area = dsc->scissor_area;
    lv_area_move(&scissor_area, layer_start_x, layer_start_y);

    nema_set_clip(scissor_area.x1, scissor_area.y1, 
                lv_area_get_width(&scissor_area), 
                lv_area_get_height(&scissor_area));

    /* clear area */
    if(!path) {
        /* clear color needs to ignore fill_dsc.opa */

        uint32_t clear_color = nema_rgba(dsc->fill_dsc.color.red, dsc->fill_dsc.color.green, dsc->fill_dsc.color.blue, LV_OPA_COVER);
        nema_clear(clear_color);
        LV_PROFILER_END;
        return;
    }

    /* covert blend mode*/
    uint32_t blend = lv_vector_blend_to_nema(dsc->blend_mode);
    if(blend == 0xffffffff) {
        LV_LOG_WARN("unsupported blend mode: %d, use NEMA_BL_SRC_OVER instead.", dsc->blend_mode);
        blend = NEMA_BL_SRC_OVER;
        return;
    }

    /* set path quality */
    uint32_t quality = lv_vector_quality_to_nema(path->quality);
    nema_vg_set_quality(quality);

    /*clear path*/
    nema_vg_path_clear(unit->vg_path);

    /* convert path */
    lv_array_t vg_path_seg;
    lv_vector_path_to_nema(&vg_path_seg, &path->ops);
    nema_vg_path_set_shape(unit->vg_path, vg_path_seg.size, vg_path_seg.data, 
                            path->points.size*2, (nema_vg_float_t*)path->points.data);

    /*set path matrix*/
    nema_vg_path_set_matrix(unit->vg_path, (nema_matrix3x3_t*)&dsc->matrix.m);

    /* fill  */
    if(dsc->fill_dsc.opa != LV_OPA_TRANSP) {
        lv_vector_paint_to_nema(unit->vg_paint, unit->vg_grad, &dsc->fill_dsc);
        nema_vg_draw_path(unit->vg_path, unit->vg_paint);
    }

    /* draw stroke */
    if(dsc->stroke_dsc.opa != LV_OPA_TRANSP) {
        lv_vector_stroke_to_nema(unit->vg_paint, unit->vg_grad, &dsc->stroke_dsc);
        nema_vg_draw_path(unit->vg_path, unit->vg_paint);
    }

    /* free the allocated memory*/
    nema_img_obj_t * ptr_img_obj;
    nema_img_obj_t * ptr_palette_obj

    lv_ambiq_get_vg_paint_tex(unit->vg_paint, &ptr_img_obj, &ptr_palette_obj)
    if(ptr_img_obj)
    {
        lv_free(ptr_img_obj);
    }

    if(ptr_palette_obj)
    {
        lv_free(ptr_palette_obj);
    }

    lv_array_deinit(&vg_path_seg);



    LV_PROFILER_END;
}

#endif /*LV_USE_DRAW_AMBIQ && LV_USE_VECTOR_GRAPHIC*/
