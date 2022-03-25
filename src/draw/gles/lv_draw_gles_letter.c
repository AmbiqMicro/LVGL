/**
 * @file lv_draw_gles_letter.c
 *
 */


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
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      INCLUDES
 *********************/

#include "../../lv_conf_internal.h"


#if LV_USE_GPU_GLES

#include "../lv_draw.h"
#include "lv_draw_gles_utils.h"



#include LV_GPU_GLES_GLAD_INCLUDE_PATH

/*********************
 *      DEFINES
 *********************/

void lv_draw_sw_letter(lv_draw_ctx_t * draw_ctx, const lv_draw_label_dsc_t * dsc,  const lv_point_t * pos_p,
                       uint32_t letter);

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_draw_gles_draw_letter(lv_draw_ctx_t * draw_ctx, const lv_draw_label_dsc_t * dsc,  const lv_point_t * pos_p,
                              uint32_t letter)
{
#if LV_USE_GPU_GLES_SW_MIXED
    lv_draw_sw_letter(draw_ctx, dsc, pos_p, letter);
    lv_draw_gles_utils_upload_texture(draw_ctx);
#endif /* LV_USE_GPU_GLES_SW_MIXED */
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_GPU_GLES*/