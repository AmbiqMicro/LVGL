/**
 * @file lv_draw_ambiq_img.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../misc/lv_area_private.h"
#include "../lv_image_decoder_private.h"
#include "../lv_draw_image_private.h"
#include "../lv_draw_private.h"

#if LV_USE_DRAW_AMBIQ
#include "lv_draw_ambiq.h"
#include "../../misc/lv_log.h"
#include "../../core/lv_refr_private.h"
#include "../../stdlib/lv_mem.h"
#include "../../misc/lv_math.h"
#include "../../misc/lv_color.h"
#include "../../stdlib/lv_string.h"
#include "../../core/lv_global.h"
#include "lv_draw_ambiq_color.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_draw_ambiq_image_core(lv_draw_unit_t * draw_unit,
                                    const lv_draw_image_dsc_t * draw_dsc,
                                    lv_image_decoder_dsc_t * decoder_dsc,
                                    const lv_area_t * coords, 
                                    const lv_draw_buf_t* mask_image);

/**********************
 *  STATIC VARIABLES
 **********************/
#define _draw_info LV_GLOBAL_DEFAULT()->draw_info

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_ambiq_layer(lv_draw_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc, const lv_area_t * coords)
{
    lv_layer_t * layer_to_draw = (lv_layer_t *)draw_dsc->src;

    /*It can happen that nothing was draw on a layer and therefore its buffer is not allocated.
     *In this case just return. */
    if(layer_to_draw->draw_buf == NULL) return;

    lv_draw_image_dsc_t new_draw_dsc = *draw_dsc;
    new_draw_dsc.src = layer_to_draw->draw_buf;
    lv_draw_ambiq_image(draw_unit, &new_draw_dsc, coords);
}

void lv_draw_ambiq_image(lv_draw_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc,
                      const lv_area_t * coords)
{
    lv_image_decoder_dsc_t decoder_dsc;
    lv_result_t res = lv_image_decoder_open(&decoder_dsc, draw_dsc->src, NULL);
    if(res != LV_RESULT_OK) {
        LV_LOG_ERROR("Failed to open image");
        return;
    }

    /*The whole image is not available, we can't draw it with GPU*/
    if(decoder_dsc.decoded == NULL) {
        LV_LOG_WARN("Ambiq GPU needs to load the whole image to GPU accessible RAM.\n");
        return;
    }


    const lv_draw_buf_t * mask_img = NULL;
    lv_image_decoder_dsc_t mask_decoder_dsc;
    bool need_release_mask_decoder = false;
    if(draw_dsc->bitmap_mask_src)
    {
        lv_result_t decoder_res = lv_image_decoder_open(&mask_decoder_dsc, draw_dsc->bitmap_mask_src, NULL);
        if(decoder_res == LV_RESULT_OK)
        {
            if(mask_decoder_dsc.decoded) {
                if(mask_decoder_dsc.decoded->header.cf == LV_COLOR_FORMAT_A8 ||
                   mask_decoder_dsc.decoded->header.cf == LV_COLOR_FORMAT_L8) {

                    if((mask_decoder_dsc.decoded->header.w == draw_dsc->header.w) && 
                       (mask_decoder_dsc.decoded->header.h == draw_dsc->header.h))
                    {
                        mask_img = mask_decoder_dsc.decoded;
                    }
                    else
                    {
                        LV_LOG_WARN("GPU limitation, mask size should be same as the texture size or the framebuffer size!");
                    }
                }
                else {
                    LV_LOG_WARN("The mask image is not A8/L8 format. Drawing the image without mask.");
                }
            }
            else {
                LV_LOG_WARN("Couldn't decode the mask image. Drawing the image without mask.");
            }

            need_release_mask_decoder = true;
        }
        else {
            LV_LOG_WARN("MASK image decode failed. Drawing the image without mask.");
        }
    }

    if(draw_dsc->tile)
    {
        if(draw_dsc->rotation 
        || draw_dsc->scale_x != LV_SCALE_NONE || draw_dsc->scale_y != LV_SCALE_NONE
        || draw_dsc->skew_x || draw_dsc->skew_y) 
        {
             LV_LOG_WARN("Set rotation/scale/skew effect to tile image is not supported! We will ignore these parameters.");
        }

        int32_t img_w = draw_dsc->header.w;
        int32_t img_h = draw_dsc->header.h;

        bool width_is_power_of_2 = (img_w > 0) && ((img_w & (img_w - 1)) == 0);
        bool hight_is_power_of_2 = (img_h > 0) && ((img_h & (img_h - 1)) == 0);

        if( !width_is_power_of_2 || !hight_is_power_of_2 )
        {
            LV_LOG_WARN("The hight and width of tile texture is not power of 2, draw it one by one! ");
        }

        if(draw_dsc->clip_radius != 0)
        {
            LV_LOG_WARN("Set clip_radius parameter to a tiled image is not supported! ignore it! ");
        }

        if(draw_dsc->bitmap_mask_src)
        {
            LV_LOG_WARN("Set mask to a tiled image is not supported! ignore it! ");
        }
    }

        lv_area_t draw_area;
        lv_area_copy(&draw_area, coords);
        if(draw_dsc->rotation || draw_dsc->scale_x != LV_SCALE_NONE 
            || draw_dsc->scale_y != LV_SCALE_NONE) {
            int32_t w = lv_area_get_width(coords);
            int32_t h = lv_area_get_height(coords);

            lv_image_buf_get_transformed_area(&draw_area, w, h, 
                                              draw_dsc->rotation, 
                                              draw_dsc->scale_x, draw_dsc->scale_y,
                                              &draw_dsc->pivot);

            draw_area.x1 += coords->x1;
            draw_area.y1 += coords->y1;
            draw_area.x2 += coords->x1;
            draw_area.y2 += coords->y1;
        }

        lv_area_t clipped_img_area;
        if(!lv_area_intersect(&clipped_img_area, &draw_area, draw_unit->clip_area)) {
            return;
        }

        // TODO: compare the draw_area min and max value with coordinate range limitation.

        lv_draw_ambiq_image_core(draw_unit, draw_dsc, &decoder_dsc, 
                                 coords, mask_img);

    lv_image_decoder_close(&decoder_dsc);
    if(need_release_mask_decoder) lv_image_decoder_close(&mask_decoder_dsc);
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_draw_ambiq_image_core(lv_draw_unit_t * draw_unit,
                                    const lv_draw_image_dsc_t * draw_dsc,
                                    lv_image_decoder_dsc_t * decoder_dsc,
                                    const lv_area_t * coords, 
                                    const lv_draw_buf_t* mask_image)
{

    nema_tex_format_t nema_cf = lv_ambiq_color_format_map_src(draw_dsc->header.cf);
    if(nema_cf == COLOR_FORMAT_INVALID)
    {
        LV_LOG_WARN("GPU failed, not supported color format!");
        return;
    }

    //Set blend mode
    if((draw_dsc->blend_mode == LV_BLEND_MODE_SUBTRACTIVE) || 
       (draw_dsc->blend_mode == LV_BLEND_MODE_MULTIPLY))
    {
        LV_LOG_WARN("GPU failed, not supported blend mode!");
        return;  
    }

    lv_layer_t * layer = draw_unit->target_layer;

    uint32_t recolor_rgba =lv_ambiq_color_convert(draw_dsc->recolor, draw_dsc->recolor_opa);


    // handle blend mode

    uint32_t blending_mode;

    switch(draw_dsc->blend_mode)
    {
        case LV_BLEND_MODE_ADDITIVE:
            blending_mode = NEMA_BL_ADD;
            break;
        default:
            if(layer->color_format == LV_COLOR_FORMAT_ARGB8888)
            {
                blending_mode = NEMA_BL_SRC_OVER|NEMA_BLOP_SRC_PREMULT;
            }
            else
            {
                blending_mode = NEMA_BL_SIMPLE;
            }           
            break;
    }

    if(draw_dsc->opa < LV_OPA_MAX)
    {
        blending_mode |= NEMA_BLOP_MODULATE_A;

        uint32_t global_opa = draw_dsc->opa;
        nema_set_const_color(global_opa<<24);
    }

    // handle look up table(LUT) color format
    if((draw_dsc->header.cf == LV_COLOR_FORMAT_I1) ||
    (draw_dsc->header.cf == LV_COLOR_FORMAT_I2) ||
    (draw_dsc->header.cf == LV_COLOR_FORMAT_I4) ||
    (draw_dsc->header.cf == LV_COLOR_FORMAT_I8))
    {
        blending_mode |= NEMA_BLOP_LUT;

        uint32_t lut_size;
        switch(draw_dsc->header.cf) {
            case NEMA_L1:
            case NEMA_L1LE:
                lut_size = 2U;
                break;
            case NEMA_L2:
            case NEMA_L2LE:
                lut_size = 4U;
                break;
            case NEMA_L4:
            case NEMA_L4LE:
                lut_size = 16U;
                break;
            // case NEMA_L8:
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
    if((draw_dsc->header.cf == LV_COLOR_FORMAT_A1) ||
    (draw_dsc->header.cf == LV_COLOR_FORMAT_A2) ||
    (draw_dsc->header.cf == LV_COLOR_FORMAT_A4) ||
    (draw_dsc->header.cf == LV_COLOR_FORMAT_A8))
    {
        nema_set_tex_color(recolor_rgba);
    }
    else
    {
        nema_set_tex_color(0x0);
    }

    // handle mask
    if((mask_image) || (draw_dsc->header.cf == LV_COLOR_FORMAT_RGB565A8))
    {

        if(mask_image && draw_dsc->header.cf != LV_COLOR_FORMAT_RGB565A8)
        {
            nema_bind_tex(NEMA_TEX3,
                          (uintptr_t)mask_image->data,
                          mask_image->header.w,
                          mask_image->header.h,
                          NEMA_A8,
                          -1,
                          NEMA_TEX_BORDER);

        }
        else if(!mask_image && draw_dsc->header.cf == LV_COLOR_FORMAT_RGB565A8)
        {
            nema_bind_tex(NEMA_TEX3,
                          (uintptr_t)(decoder_dsc->decoded->data + draw_dsc->header.h*draw_dsc->header.stride),
                          draw_dsc->header.w,
                          draw_dsc->header.h,
                          NEMA_A8,
                          -1,
                          NEMA_TEX_BORDER);
        }
        else if(mask_image && draw_dsc->header.cf == LV_COLOR_FORMAT_RGB565A8)
        {
            //TODO:
            //malloc a buffer
            //bind the allocated buffer to TEX3
            // set clip area
            //bind the the alpha part of RGB565A8 to TEX2
            //set blend, copy TEX2->TEX3
            //bind the mask buffer to TEX 2
            // set blend, make TEX2*TEX3->TEX3
        
        }

        blending_mode |= NEMA_BLOP_STENCIL_TXTY;
    }

    // handle tile image
    bool tile_draw_one_by_one = false;
    if(draw_dsc->tile)
    {
        int32_t img_w = draw_dsc->header.w;
        int32_t img_h = draw_dsc->header.h;

        bool width_is_power_of_2 = (img_w > 0) && ((img_w & (img_w - 1)) == 0);
        bool hight_is_power_of_2 = (img_h > 0) && ((img_h & (img_h - 1)) == 0);

        if( !width_is_power_of_2 || !hight_is_power_of_2 )
        {
            tile_draw_one_by_one = true;
        }
    }

    // handle AA
    //Previous AA setting.
    uint32_t prev_aa = 0xFFFFFFFF;

    //Set antialias
    if(draw_dsc->antialias)
    {
        prev_aa = nema_enable_aa(true, true, true, true);
    }
    else
    {
        prev_aa = nema_enable_aa(false, false, false, false);
    }

    if(draw_dsc->recolor_opa > LV_OPA_MIN)
    {
        blending_mode |= NEMA_BLOP_RECOLOR;
        nema_set_recolor_color(recolor_rgba);
    }

    //bind image
    uint32_t tex_wrap_mode = (draw_dsc->tile && !tile_draw_one_by_one) ? NEMA_TEX_REPEAT : NEMA_TEX_BORDER;
    nema_bind_tex(NEMA_TEX1,
                (uintptr_t)decoder_dsc->decoded->data,
                draw_dsc->header.w,
                draw_dsc->header.h,
                nema_cf,
                -1,
                NEMA_FILTER_BL|tex_wrap_mode);
    

    nema_set_blend(blending_mode, NEMA_TEX0, NEMA_TEX1, NEMA_NOTEX);

    bool transformed = draw_dsc->rotation != 0 || draw_dsc->scale_x != LV_SCALE_NONE ||
                       draw_dsc->scale_y != LV_SCALE_NONE || draw_dsc->skew_y != 0 || draw_dsc->skew_x != 0 ? true : false;

    if(!transformed)
    {
        //Blit
        if(!tile_draw_one_by_one){
            nema_blit(coords->x1 - layer->buf_area.x1, coords->y1 - layer->buf_area.y1);
        }
        else{
            lv_area_t tile_area;
            uint32_t img_w = draw_dsc->header.w;
            uint32_t img_h = draw_dsc->header.h;
            if(lv_area_get_width(&draw_dsc->image_area) >= 0) {
                tile_area = draw_dsc->image_area;
            }
            else {
                tile_area = *coords;
            }
            lv_area_set_width(&tile_area, img_w);
            lv_area_set_height(&tile_area, img_h);

            int32_t tile_x_start = tile_area.x1;

            while(tile_area.y1 <= coords->y2) {
                while(tile_area.x1 <= coords->x2) {

                    lv_area_t clipped_img_area;
                    if(lv_area_intersect(&clipped_img_area, &tile_area, coords)) {
                        nema_blit(tile_area.x1 - layer->buf_area.x1, tile_area.y1 - layer->buf_area.y1);
                    }

                    tile_area.x1 += img_w;
                    tile_area.x2 += img_w;
                }

                tile_area.y1 += img_h;
                tile_area.y2 += img_h;
                tile_area.x1 = tile_x_start;
                tile_area.x2 = tile_x_start + img_w - 1;
            }           
        }
    }
    else
    {

        // handle rotation

        //calculate rotation matrix
        nema_matrix3x3_t m;
        nema_mat3x3_load_identity(m);
        nema_mat3x3_translate(m, - draw_dsc->pivot.x, - draw_dsc->pivot.y);
        nema_mat3x3_scale(m, draw_dsc->scale_x/256.f, draw_dsc->scale_y/256.f);
        nema_mat3x3_rotate(m, draw_dsc->rotation/10.f);
        nema_mat3x3_shear(m, draw_dsc->skew_x/10.f, draw_dsc->skew_y/10.f);
        nema_mat3x3_translate(m, draw_dsc->pivot.x, draw_dsc->pivot.x);

        nema_matrix3x3_t m_draw;
        nema_mat3x3_copy(m_draw, m);


        //handle the special case when the image coordinate is not the same as draw coordinate
        nema_mat3x3_translate(m, draw_dsc->image_area.x1  - layer->buf_area.x1, draw_dsc->image_area.y1  - layer->buf_area.y1); 
        nema_mat3x3_invert(m);
        nema_set_matrix(m);



        //rotate points
        nema_mat3x3_translate(m_draw, coords->x1  - layer->buf_area.x1, coords->y1  - layer->buf_area.y1); 
        
        float x0 = 0;
        float y0 = 0;
        float x1 = x0 + lv_area_get_width(coords);
        float y1 = y0;
        float x2 = x0 + lv_area_get_width(coords);
        float y2 = y0 + lv_area_get_height(coords);
        float x3 = x0;
        float y3 = y0 + lv_area_get_height(coords);

        nema_mat3x3_mul_vec(m_draw, &x0, &y0);
        nema_mat3x3_mul_vec(m_draw, &x1, &y1);
        nema_mat3x3_mul_vec(m_draw, &x2, &y2);
        nema_mat3x3_mul_vec(m_draw, &x3, &y3);

        //draw
        nema_raster_quad_f(x0, y0,
                           x1, y1,
                           x2, y2,
                           x3, y3);
    }




    // handle clip radius(TODO)
    // handle recolor(TODO)
    // enable tilling when rotation(TODO)
    // LUT texture may not supported.
    // AL88 format is this same as NEMA_AL88?

    
    //Set antialias
    if(draw_dsc->antialias)
    {
        nema_enable_aa_flags(prev_aa);
    }
    
}


#endif /*LV_USE_DRAW_AMBIQ*/
