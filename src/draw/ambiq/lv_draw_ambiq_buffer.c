/**
 * @file lv_draw_ambiq_buffer.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw_buf_private.h"
#include "../../misc/lv_types.h"
#include "../../stdlib/lv_string.h"
#include "../../core/lv_global.h"
#include "../../misc/lv_math.h"
#include "../../misc/lv_area_private.h"

#if LV_USE_DRAW_AMBIQ
#include "lv_draw_ambiq.h"
#include "../../core/lv_refr.h"
#include "../../misc/lv_assert.h"


/*********************
 *      DEFINES
 *********************/
#define default_handlers LV_GLOBAL_DEFAULT()->draw_buf_handlers
#define font_draw_buf_handlers LV_GLOBAL_DEFAULT()->font_draw_buf_handlers
#define image_cache_draw_buf_handlers LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers


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

static inline void * lv_draw_ambiq_buffer_malloc_core(uint32_t pool, size_t size, lv_color_format_t color_format)
{
    LV_UNUSED(color_format);
    nema_buffer_t buf;
    buf = nema_buffer_create_pool(pool, size);
    return buf.base_virt;
}

static inline void lv_draw_ambiq_buffer_free_core(uint32_t pool, void * buf)
{
    nema_buffer_t nema_buf = {
        .base_virt = buf,
        .base_phys = (uintptr_t)buf,
        .fd = pool,
        .size = 0,/*not used in nema_buffer_destroy API*/
    };

    LV_ASSERT_MSG(nema_buffer_is_within_pool(pool, (uint32_t)buf, 0), "Buffer is not within the pool");
    nema_buffer_destroy(&nema_buf);
}

static void * lv_draw_ambiq_buffer_align(void * buf, lv_color_format_t color_format)
{
    LV_UNUSED(color_format);

    uint8_t * buf_u8 = buf;
    if(buf_u8) {
        buf_u8 = (uint8_t *)LV_ROUND_UP((lv_uintptr_t)buf_u8, LV_DRAW_BUF_ALIGN);
    }
    return buf_u8;
}

static uint32_t lv_draw_ambiq_buffer_width_to_stride(uint32_t w, lv_color_format_t color_format)
{
    uint32_t width_byte;
    width_byte = w * lv_color_format_get_bpp(color_format);
    width_byte = (width_byte + 7) >> 3; /*Round up*/

    return LV_ROUND_UP(width_byte, LV_DRAW_BUF_STRIDE_ALIGN);
}

static inline void lv_draw_ambiq_buffer_flush_core(uint32_t pool, const lv_draw_buf_t * draw_buf, const lv_area_t * area)
{
    nema_buffer_t nema_buf = {
        .base_virt = draw_buf->data,
        .base_phys = (uintptr_t)draw_buf->data,
        .fd = pool,
        .size = draw_buf->data_size,
    };

    LV_ASSERT_MSG(nema_buffer_is_within_pool(pool, (uint32_t)draw_buf->data, draw_buf->data_size), "Buffer is not within the pool");
    nema_buffer_flush(&nema_buf);   
}

static void lv_draw_ambiq_buffer_invalidate_core(uint32_t pool, const lv_draw_buf_t * draw_buf, const lv_area_t * area)
{
    nema_buffer_t nema_buf = {
        .base_virt = draw_buf->data,
        .base_phys = (uintptr_t)draw_buf->data,
        .fd = pool,
        .size = draw_buf->data_size,
    };

    LV_ASSERT_MSG(nema_buffer_is_within_pool(pool, (uint32_t)draw_buf->data, draw_buf->data_size), "Buffer is not within the pool");
    nema_buffer_invalidate(&nema_buf);   
}




static void lv_draw_ambiq_buffer_clean(lv_draw_buf_t * draw_buf, const lv_area_t* a)
{
    const lv_image_header_t * header = &draw_buf->header;

    lv_area_t a_draw_buf;
    a_draw_buf.x1 = 0;
    a_draw_buf.y1 = 0;
    a_draw_buf.x2 = draw_buf->header.w - 1;
    a_draw_buf.y2 = draw_buf->header.h - 1;

    lv_area_t a_clipped;
    if(a == NULL) {
        a_clipped = a_draw_buf;
    } else {
        if(!lv_area_intersect(&a_clipped, a, &a_draw_buf)) return;
    }

    if(lv_area_get_width(&a_clipped) <= 0) return;
    if(lv_area_get_height(&a_clipped) <= 0) return;



    nema_cmdlist_t cl = nema_cl_create_sized(512);

    lv_draw_ambiq_nema_context_lock();
    nema_cl_bind(&cl);
    nema_cl_rewind(&cl);

    bool cl_need_submit = true;

    nema_tex_format_t des_format = lv_ambiq_color_format_map_des(header->cf);
    uint32_t palette_size = 0;
    if(des_format != COLOR_FORMAT_INVALID) {

        nema_bind_dst_tex((uintptr_t)draw_buf->data, header->w, header->h, des_format, header->stride);
        nema_set_clip(a_clipped.x1, a_clipped.y1, lv_area_get_width(&a_clipped), lv_area_get_height(&a_clipped));
        nema_clear(0x0);
    }
    else if(header->cf == LV_COLOR_FORMAT_RGB565A8)
    {
        nema_bind_dst_tex((uintptr_t)draw_buf->data, header->w, header->h, NEMA_RGB565, -1);
        nema_set_clip(a_clipped.x1, a_clipped.y1, lv_area_get_width(&a_clipped), lv_area_get_height(&a_clipped));
        nema_clear(0x0);

        nema_bind_dst_tex((uintptr_t)draw_buf->data + header->w * header->h * 2, header->w, header->h, NEMA_A8, -1);
        nema_set_clip(a_clipped.x1, a_clipped.y1, lv_area_get_width(&a_clipped), lv_area_get_height(&a_clipped));
        nema_clear(0x0);      
    }
    else if(header->cf == LV_COLOR_FORMAT_I1 || header->cf == LV_COLOR_FORMAT_I2 || header->cf == LV_COLOR_FORMAT_I4 || header->cf == LV_COLOR_FORMAT_I8)
    {
        switch (header->cf)
        {
            case LV_COLOR_FORMAT_I1:
                palette_size = 2;
                des_format = NEMA_L1;
                break;
            case LV_COLOR_FORMAT_I2:
                palette_size = 4;
                des_format = NEMA_L2;
                break;
            case LV_COLOR_FORMAT_I4:
                palette_size = 16;
                des_format = NEMA_L4;
                break;
            case LV_COLOR_FORMAT_I8:
                palette_size = 256;
                des_format = NEMA_L8;
                break;
        }

        if(a == NULL)
        {
            nema_bind_dst_tex((uintptr_t)draw_buf->data, palette_size, 1, NEMA_RGBA8888, -1);
            nema_set_clip(0, 0, palette_size, 1);
            nema_clear(0x0);
        }

        nema_bind_dst_tex((uintptr_t)draw_buf->data + palette_size * 4, header->w, header->h, des_format, -1);
        nema_set_clip(a_clipped.x1, a_clipped.y1, lv_area_get_width(&a_clipped), lv_area_get_height(&a_clipped));
        nema_clear(0x0);
    }
    else
    {
        LV_LOG_WARN("Unsupported color format");
        cl_need_submit = false;
    }
    
    if(cl_need_submit) {
        nema_cl_submit(&cl);
    }

    lv_draw_ambiq_nema_context_unlock();

    uint32_t nema_error_code = nema_get_error();
    if(nema_error_code != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("Nema error: 0x%08lX", nema_error_code);
    }

    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);
}

static void lv_draw_ambiq_buffer_copy(lv_draw_buf_t * dest, const lv_area_t * dest_area,
                      const lv_draw_buf_t * src, const lv_area_t * src_area)
{
    /*Using the GPU for color format conversion is straightforward, 
    but we aim to avoid handling too many corner cases in this context.*/
    LV_ASSERT_FORMAT_MSG(dest->header.cf == src->header.cf, "Color format mismatch: %d != %d",
                         dest->header.cf, src->header.cf);

    const lv_image_header_t * dest_header = &dest->header;
    const lv_image_header_t * src_header = &src->header;
    int32_t start_x_src, start_y_src;
    int32_t start_x_dest, start_y_dest;
    int32_t line_width_dest, line_hight_dest;
    int32_t line_width_src, line_hight_src;

    if(dest_area == NULL) 
    {
        line_width_dest = dest_header->w;
        line_hight_dest = dest_header->h;
        start_x_dest = 0;
        start_y_dest = 0;
    }
    else
    {
        line_width_dest = lv_area_get_width(dest_area);
        line_hight_dest = lv_area_get_height(dest_area);
        start_x_dest = dest_area->x1;
        start_y_dest = dest_area->y1;
    }

    if(src_area == NULL) 
    {
        line_width_src = src_header->w;
        line_hight_src = src_header->h;
        start_x_src = 0;
        start_y_src = 0;
    }
    else
    {
        line_width_src = lv_area_get_width(src_area);
        line_hight_src = lv_area_get_height(src_area);
        start_x_src = src_area->x1;
        start_y_src = src_area->y1;
    }

    /*Check source and dest area have same width and hight*/
    LV_ASSERT_MSG(((line_width_src == line_width_dest) && (line_hight_src == line_hight_dest)), "Source and destination areas have different width and hight");

    nema_cmdlist_t cl = nema_cl_create_sized(0x100);
    LV_ASSERT_NULL(cl.bo.base_virt);
    if(cl.bo.base_virt == NULL) return;

    lv_draw_ambiq_nema_context_lock();
    nema_cl_bind(&cl);
    nema_cl_rewind(&cl);

    bool cl_need_submit = true;
    nema_tex_format_t des_format = lv_ambiq_color_format_map_des(dest_header->cf);
    uint32_t palette_size = 0;
    if(des_format != COLOR_FORMAT_INVALID) {

        nema_bind_dst_tex((uintptr_t)dest->data, dest_header->w, dest_header->h, des_format, -1);
        nema_set_clip(start_x_dest, start_y_dest, line_width_dest, line_hight_dest);
        nema_bind_src_tex((uintptr_t)src->data, src_header->w, src_header->h, des_format, -1, NEMA_FILTER_PS);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit_subrect(start_x_dest, start_y_dest, line_width_src, line_hight_src, start_x_src, start_y_src);
    }
    else if(dest_header->cf == LV_COLOR_FORMAT_RGB565A8)
    {
        nema_bind_dst_tex((uintptr_t)dest->data, dest_header->w, dest_header->h, NEMA_RGB565, -1);
        nema_set_clip(start_x_dest, start_y_dest, line_width_dest, line_hight_dest);
        nema_bind_src_tex((uintptr_t)src->data, src_header->w, src_header->h, NEMA_RGB565, -1, NEMA_FILTER_PS);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit_subrect(start_x_dest, start_y_dest, line_width_src, line_hight_src, start_x_src, start_y_src);

        nema_bind_dst_tex((uintptr_t)dest->data + dest_header->w * dest_header->h * 2, dest_header->w, dest_header->h, NEMA_A8, -1);
        nema_set_clip(start_x_dest, start_y_dest, line_width_dest, line_hight_dest);
        nema_bind_src_tex((uintptr_t)src->data + src_header->w * src_header->h * 2, src_header->w, src_header->h, NEMA_A8, -1, NEMA_FILTER_PS);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit_subrect(start_x_dest, start_y_dest, line_width_src, line_hight_src, start_x_src, start_y_src);
    }
    else if(dest_header->cf == LV_COLOR_FORMAT_I1 || dest_header->cf == LV_COLOR_FORMAT_I2 || dest_header->cf == LV_COLOR_FORMAT_I4 || dest_header->cf == LV_COLOR_FORMAT_I8)
    {
        switch (dest_header->cf)
        {
            case LV_COLOR_FORMAT_I1:
                palette_size = 2;
                des_format = NEMA_L1;
                break;
            case LV_COLOR_FORMAT_I2:
                palette_size = 4;
                des_format = NEMA_L2;
                break;
            case LV_COLOR_FORMAT_I4:
                palette_size = 16;
                des_format = NEMA_L4;
                break;
            case LV_COLOR_FORMAT_I8:
                palette_size = 256;
                des_format = NEMA_L8;
                break;
        }

        if(dest_area == NULL && src_area == NULL)
        {
            nema_bind_dst_tex((uintptr_t)dest->data, palette_size, 1, NEMA_RGBA8888, -1);
            nema_set_clip(0, 0, palette_size, 1);
            nema_bind_src_tex((uintptr_t)src->data, palette_size, 1, NEMA_RGBA8888, -1, NEMA_FILTER_PS);
            nema_set_blend_blit(NEMA_BL_SRC);
            nema_blit(0, 0);
        }

        nema_bind_dst_tex((uintptr_t)dest->data + palette_size * 4, dest_header->w, dest_header->h, des_format, -1);
        nema_set_clip(start_x_dest, start_y_dest, line_width_dest, line_hight_dest);
        nema_bind_src_tex((uintptr_t)src->data + palette_size * 4, src_header->w, src_header->h, des_format, -1, NEMA_FILTER_PS);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit_subrect(start_x_dest, start_y_dest, line_width_src, line_hight_src, start_x_src, start_y_src);
    }
    else
    {
        LV_LOG_WARN("Unsupported color format");
        cl_need_submit = false;
    }

    
    if(cl_need_submit) {
        nema_cl_submit(&cl);
    }

    lv_draw_ambiq_nema_context_unlock();

    uint32_t nema_error_code = nema_get_error();
    if(nema_error_code != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("Nema error: 0x%08lX", nema_error_code);
    }

    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);
}

static void* lv_draw_ambiq_buffer_malloc(size_t size, lv_color_format_t color_format)
{
    return lv_draw_ambiq_buffer_malloc_core(NEMA_MEM_POOL_FB, size, color_format);
}

static void lv_draw_ambiq_buffer_free(void * buf)
{
    lv_draw_ambiq_buffer_free_core(NEMA_MEM_POOL_FB, buf);
}

static void lv_draw_ambiq_buffer_flush(const lv_draw_buf_t * draw_buf, const lv_area_t * area)
{
    lv_draw_ambiq_buffer_flush_core(NEMA_MEM_POOL_FB, draw_buf, area);
}

static void lv_draw_ambiq_buffer_invalidate(const lv_draw_buf_t * draw_buf, const lv_area_t * area)
{
    lv_draw_ambiq_buffer_invalidate_core(NEMA_MEM_POOL_FB, draw_buf, area);
}

static void* lv_draw_ambiq_buffer_malloc_texture(size_t size, lv_color_format_t color_format)
{
    return lv_draw_ambiq_buffer_malloc_core(NEMA_MEM_POOL_ASSETS, size, color_format);
}

static void lv_draw_ambiq_buffer_free_texture(void * buf)
{
    lv_draw_ambiq_buffer_free_core(NEMA_MEM_POOL_ASSETS, buf);
}

static void lv_draw_ambiq_buffer_flush_texture(const lv_draw_buf_t * draw_buf, const lv_area_t * area)
{
    lv_draw_ambiq_buffer_flush_core(NEMA_MEM_POOL_ASSETS, draw_buf, area);
}

static void lv_draw_ambiq_buffer_invalidate_texture(const lv_draw_buf_t * draw_buf, const lv_area_t * area)
{
    lv_draw_ambiq_buffer_invalidate_core(NEMA_MEM_POOL_ASSETS, draw_buf, area);
}



void lv_draw_ambiq_init_buf_handlers(void)
{
    default_handlers.buf_malloc_cb = lv_draw_ambiq_buffer_malloc;
    default_handlers.buf_free_cb = lv_draw_ambiq_buffer_free;
    default_handlers.flush_cache_cb = lv_draw_ambiq_buffer_flush;
    default_handlers.invalidate_cache_cb = lv_draw_ambiq_buffer_invalidate;
    default_handlers.align_pointer_cb = lv_draw_ambiq_buffer_align;
    default_handlers.width_to_stride_cb = lv_draw_ambiq_buffer_width_to_stride;
    default_handlers.clear_cb = lv_draw_ambiq_buffer_clean;
    default_handlers.copy_cb = lv_draw_ambiq_buffer_copy;

    image_cache_draw_buf_handlers.buf_malloc_cb = lv_draw_ambiq_buffer_malloc_texture;
    image_cache_draw_buf_handlers.buf_free_cb = lv_draw_ambiq_buffer_free_texture;
    image_cache_draw_buf_handlers.flush_cache_cb = lv_draw_ambiq_buffer_flush_texture;
    image_cache_draw_buf_handlers.invalidate_cache_cb = lv_draw_ambiq_buffer_invalidate_texture;
    image_cache_draw_buf_handlers.align_pointer_cb = lv_draw_ambiq_buffer_align;
    image_cache_draw_buf_handlers.width_to_stride_cb = lv_draw_ambiq_buffer_width_to_stride;
    image_cache_draw_buf_handlers.clear_cb = lv_draw_ambiq_buffer_clean;
    image_cache_draw_buf_handlers.copy_cb = lv_draw_ambiq_buffer_copy;

    font_draw_buf_handlers.buf_malloc_cb = lv_draw_ambiq_buffer_malloc_texture;
    font_draw_buf_handlers.buf_free_cb = lv_draw_ambiq_buffer_free_texture;
    font_draw_buf_handlers.flush_cache_cb = lv_draw_ambiq_buffer_flush_texture;
    font_draw_buf_handlers.invalidate_cache_cb = lv_draw_ambiq_buffer_invalidate_texture;
    font_draw_buf_handlers.align_pointer_cb = lv_draw_ambiq_buffer_align;
    font_draw_buf_handlers.width_to_stride_cb = lv_draw_ambiq_buffer_width_to_stride;
    font_draw_buf_handlers.clear_cb = lv_draw_ambiq_buffer_clean;
    font_draw_buf_handlers.copy_cb = lv_draw_ambiq_buffer_copy;

}


#endif /*LV_USE_DRAW_AMBIQ*/
