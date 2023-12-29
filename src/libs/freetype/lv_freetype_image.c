/**
 * @file lv_freetype_image.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_freetype_private.h"

/*********************
 *      DEFINES
 *********************/

#if LV_USE_FREETYPE && LV_FREETYPE_CACHE_TYPE == LV_FREETYPE_CACHE_TYPE_IMAGE

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_freetype_cache_context_t {
};

struct _lv_freetype_cache_node_t {
    FT_Face face;
    lv_cache_t * image_cache;
    lv_cache_t * glyph_cache;
};

typedef struct _lv_freetype_image_cache_data_t {
    FT_UInt glyph_index;
    uint32_t size;

    lv_draw_buf_t * draw_buf;
} lv_freetype_image_cache_data_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static const uint8_t * freetype_get_glyph_bitmap_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc,
                                                    uint32_t unicode_letter,
                                                    uint8_t * bitmap_out);

static bool freetype_image_create_cb(lv_freetype_image_cache_data_t * data, void * user_data);
static void freetype_image_free_cb(lv_freetype_image_cache_data_t * node, void * user_data);
static lv_cache_compare_res_t freetype_image_compare_cb(const lv_freetype_image_cache_data_t * lhs,
                                                        const lv_freetype_image_cache_data_t * rhs);

static void freetype_image_release_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_freetype_cache_context_t * lv_freetype_cache_context_create(lv_freetype_context_t * ctx)
{
    LV_UNUSED(ctx);

    lv_freetype_cache_context_t * cache_ctx = lv_malloc(sizeof(lv_freetype_cache_context_t));
    LV_ASSERT_MALLOC(cache_ctx);
    lv_memzero(cache_ctx, sizeof(lv_freetype_cache_context_t));

    return cache_ctx;
}

void lv_freetype_cache_context_delete(lv_freetype_cache_context_t * cache_ctx)
{
    LV_ASSERT_NULL(cache_ctx);
    lv_free(cache_ctx);
}

bool lv_freetype_on_font_create(lv_freetype_font_dsc_t * dsc)
{
    LV_ASSERT_FREETYPE_FONT_DSC(dsc);
    dsc->font.get_glyph_bitmap = freetype_get_glyph_bitmap_cb;
    dsc->font.release_glyph = freetype_image_release_cb;

    FT_Size ft_size = lv_freetype_lookup_size(dsc);
    if(!ft_size) {
        return false;
    }

    FT_Face face = ft_size->face;

    if(dsc->style & LV_FREETYPE_FONT_STYLE_ITALIC) {
        lv_freetype_italic_transform(face);
    }

    lv_freetype_cache_node_t * cache_node = lv_malloc_zeroed(sizeof(lv_freetype_cache_node_t));
    if(cache_node == NULL) {
        LV_LOG_ERROR("cache_node alloc failed");
        return false;
    }

    lv_cache_ops_t ops = {
        .compare_cb = (lv_cache_compare_cb_t)freetype_image_compare_cb,
        .create_cb = (lv_cache_create_cb_t)freetype_image_create_cb,
        .free_cb = (lv_cache_free_cb_t)freetype_image_free_cb,
    };

    cache_node->image_cache = lv_cache_create(&lv_cache_class_lru_rb_count, sizeof(lv_freetype_image_cache_data_t),
                                              LV_FREETYPE_CACHE_FT_OUTLINES, ops);
    cache_node->glyph_cache = lv_freetype_on_glyph_cache_create(dsc);
    if(cache_node->image_cache == NULL || cache_node->glyph_cache == NULL) {
        LV_LOG_ERROR("lv_cache_create failed");
        return NULL;
    }

    dsc->cache_node = cache_node;

    return true;
}

void lv_freetype_on_font_delete(lv_freetype_font_dsc_t * dsc)
{
    LV_ASSERT_FREETYPE_FONT_DSC(dsc);
    lv_cache_destroy(dsc->cache_node->image_cache, NULL);
    lv_freetype_on_glyph_cache_delete(dsc->cache_node->glyph_cache);
    lv_free(dsc->cache_node);
}

lv_cache_t * lv_freetype_get_glyph_cache(const lv_freetype_font_dsc_t * dsc)
{
    LV_ASSERT_FREETYPE_FONT_DSC(dsc);
    return dsc->cache_node->glyph_cache;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static const uint8_t * freetype_get_glyph_bitmap_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc,
                                                    uint32_t unicode_letter,
                                                    uint8_t * bitmap_out)
{
    LV_UNUSED(unicode_letter);
    LV_UNUSED(bitmap_out);

    lv_freetype_font_dsc_t * dsc = (lv_freetype_font_dsc_t *)font->dsc;
    LV_ASSERT_FREETYPE_FONT_DSC(dsc);

    FT_Size ft_size = lv_freetype_lookup_size(dsc);
    if(!ft_size) {
        return false;
    }

    FT_Face face = ft_size->face;
    FT_UInt charmap_index = FT_Get_Charmap_Index(face->charmap);
    FT_UInt glyph_index = FTC_CMapCache_Lookup(dsc->context->cmap_cache, dsc->face_id, charmap_index, unicode_letter);
    dsc->cache_node->face = face;

    lv_cache_t * cache = dsc->cache_node->image_cache;

    lv_freetype_image_cache_data_t search_key = {
        .glyph_index = glyph_index,
        .size = dsc->size,
    };

    lv_cache_entry_t * entry = lv_cache_acquire_or_create(cache, &search_key, dsc);

    g_dsc->entry = entry;
    lv_freetype_image_cache_data_t * cache_node = lv_cache_entry_get_data(entry);

    return cache_node->draw_buf->data;
}

static void freetype_image_release_cb(const lv_font_t * font, lv_font_glyph_dsc_t * g_dsc)
{
    LV_ASSERT_NULL(font);
    lv_freetype_font_dsc_t * dsc = (lv_freetype_font_dsc_t *)font->dsc;
    lv_cache_release(dsc->cache_node->image_cache, g_dsc->entry, NULL);
    g_dsc->entry = NULL;
}

/*-----------------
 * Cache Callbacks
 *----------------*/

static bool freetype_image_create_cb(lv_freetype_image_cache_data_t * data, void * user_data)
{
    lv_freetype_font_dsc_t * dsc = (lv_freetype_font_dsc_t *)user_data;

    FT_Error error;

    FT_Face face = dsc->cache_node->face;
    FT_Set_Pixel_Sizes(face, 0, dsc->size);
    error = FT_Load_Glyph(face, data->glyph_index,  FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL);
    if(error) {
        FT_ERROR_MSG("FT_Load_Glyph", error);
        return false;
    }
    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if(error) {
        FT_ERROR_MSG("FT_Render_Glyph", error);
        return false;
    }

    FT_Glyph glyph;
    error = FT_Get_Glyph(face->glyph, &glyph);
    if(error) {
        FT_ERROR_MSG("FT_Get_Glyph", error);
        return false;
    }

    FT_BitmapGlyph glyph_bitmap = (FT_BitmapGlyph)glyph;

    uint16_t box_h = glyph_bitmap->bitmap.rows;         /*Height of the bitmap in [px]*/
    uint16_t box_w = glyph_bitmap->bitmap.width;        /*Width of the bitmap in [px]*/

    uint32_t stride = lv_draw_buf_width_to_stride(box_w, LV_COLOR_FORMAT_A8);
    data->draw_buf = lv_draw_buf_create(box_w, box_h, LV_COLOR_FORMAT_A8, stride);

    for(int y = 0; y < box_h; ++y) {
        lv_memcpy((uint8_t *)(data->draw_buf->data) + y * stride, glyph_bitmap->bitmap.buffer + y * box_w,
                  box_w);
    }

    return true;
}
static void freetype_image_free_cb(lv_freetype_image_cache_data_t * data, void * user_data)
{
    LV_UNUSED(user_data);
    lv_draw_buf_destroy(data->draw_buf);
}
static lv_cache_compare_res_t freetype_image_compare_cb(const lv_freetype_image_cache_data_t * lhs,
                                                        const lv_freetype_image_cache_data_t * rhs)
{
    if(lhs->glyph_index != rhs->glyph_index) {
        return lhs->glyph_index > rhs->glyph_index ? 1 : -1;
    }
    if(lhs->size != rhs->size) {
        return lhs->size > rhs->size ? 1 : -1;
    }
    return 0;
}
#endif
