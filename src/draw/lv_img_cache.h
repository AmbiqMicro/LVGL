/**
 * @file lv_img_cache.h
 *
 */

#ifndef LV_IMG_CACHE_H
#define LV_IMG_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_img_decoder.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * When loading images from the network it can take a long time to download and decode the image.
 *
 * To avoid repeating this heavy load images can be cached.
 */
typedef struct {
    lv_img_dec_dsc_t dec_dsc; /**< Image information*/

    /** Count the cache entries's life. Add `time_to_open` to `life` when the entry is used.
     * Decrement all lifes by one every in every ::lv_img_cache_open.
     * If life == 0 the entry can be reused*/
    int32_t life;
} lv_img_cache_entry_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Open an image using the image decoder interface and cache it.
 * The image will be left open meaning if the image decoder open callback allocated memory then it will remain.
 * The image is closed if a new image is opened and the new image takes its place in the cache.
 * @param dsc       A pointer on decoder initialization descriptor.
 * @param dec_ctx   Optional decoder initialization context. Can be NULL
 * @return pointer to the cache entry or NULL if can open the image
 */
lv_img_cache_entry_t * lv_img_cache_open(const lv_img_dec_dsc_in_t * dsc, lv_img_dec_ctx_t * dec_ctx);

/**
 * Query the image decoder interface to find a decoder that's able to open the given source and extract the
 * image metadata. If such a decoder is found in the cache, use it, else it's not stored in the cache.
 * @param dsc       A pointer on decoder initialization descriptor.
 * @param header    On output will be filled with the image native dimension
 * @param caps      Optional parameter that'll be filled with the decoder's caps. Can be NULL
 * @param dec_ctx   Optional decoder initialization context. Can be NULL
 * @return LV_RES_OK if a decoder was able to open the image
 */
lv_res_t lv_img_cache_query(const lv_img_dec_dsc_in_t * dsc, lv_img_header_t * header, uint8_t * caps,
                            lv_img_dec_ctx_t * dec_ctx);

/**
 * Set the number of images to be cached.
 * More cached images mean more opened image at same time which might mean more memory usage.
 * E.g. if 20 PNG or JPG images are open in the RAM they consume memory while opened in the cache.
 * @param new_entry_cnt number of image to cache
 */
void lv_img_cache_set_size(uint16_t new_slot_num);

/**
 * Invalidate an image source in the cache.
 * Useful if the image source is updated therefore it needs to be cached again.
 * @param src an image source path to a file or pointer to an `lv_img_dsc_t` variable.
 */
void lv_img_cache_invalidate_src(const lv_img_src_t * src);

/**
 * Cleanup a cache entry returned by lv_img_cache_open.
 * This only happens if the image cache is disabled to avoid leaking an entry
 */
void lv_img_cache_cleanup(lv_img_cache_entry_t * entry);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_IMG_CACHE_H*/
