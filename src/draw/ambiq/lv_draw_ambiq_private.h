/**
 * @file lv_draw_ambiq_private.h
 *
 */

#ifndef LV_DRAW_AMBIQ_PRIVATE_H
#define LV_DRAW_AMBIQ_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw_private.h"
#if LV_USE_DRAW_AMBIQ


#include "../../misc/lv_area.h"
#include "../../misc/lv_color.h"
#include "../../display/lv_display.h"
#include "../../osal/lv_os.h"

#include "../lv_draw_vector.h"
#include "../lv_draw_triangle.h"
#include "../lv_draw_label.h"
#include "../lv_draw_image.h"
#include "../lv_draw_line.h"
#include "../lv_draw_arc.h"

#include "../../misc/lv_area_private.h"

#include "am_mcu_apollo.h"

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_raster.h"
#include "nema_graphics.h"
#include "nema_provisional.h"
#include "nema_error.h"
#include "nema_raster.h"
#include "nema_blender.h"
#include "nema_sys_defs.h"
#include "nema_interpolators.h"
#include "nema_matrix3x3.h"
#include "nema_programHW.h"

#if LV_USE_DRAW_AMBIQ_VG
#include "nema_vg.h"
#include "nema_vg_paint.h"
#include "nema_vg_path.h"
#include "nema_vg_font.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_context.h"
#endif

#include "gpu_patch.h"

/*********************
 *      DEFINES
 *********************/
#define COLOR_FORMAT_INVALID    (0xffffffffUL)

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_draw_unit_t base_unit;
    lv_draw_task_t * task_act;
#if LV_USE_OS
    lv_thread_sync_t sync;
    lv_thread_t thread;
    volatile bool inited;
    volatile bool exit_status;
#endif

    //! Record the size of allocated buffer.
    uint32_t total_buffer;

    //! Record total CLs created.
    uint32_t total_cl;

    //! Last CL submittion id, used for cl_wait()
    int32_t last_cl_id;

    //! CL list head
    nema_cmdlist_t* cl_head;

    //! gradient buffer
    nema_buffer_t small_texture_buffer;

    uint32_t small_texture_buffer_size_byte;

#if LV_USE_VECTOR_GRAPHIC

    //! VG path handle
    NEMA_VG_PATH_HANDLE  vg_path;

    //! VG paint handle
    NEMA_VG_PAINT_HANDLE vg_paint;

    //! VG gradient handle
    NEMA_VG_GRAD_HANDLE vg_grad;
#endif

    //! the current blend mode
    uint32_t blend_mode;

    //! current destination texture
    nema_tex_t dst_tex;
    
    //! current foreground texture
    nema_tex_t fg_tex;
    
    //! current background texture
    nema_tex_t bg_tex;

    // //! link list of inserted command lists
    // lv_ll_t inserted_cl_ll;

    //! mutex for inserted_cl_ll, prevent ll operations from different threads
    lv_mutex_t mutex_nema_context;

    uint32_t nema_context_lock_count;

} lv_draw_ambiq_unit_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

extern uint32_t nema_enable_aa_flags(uint32_t aa);


/**
 * Initialize the draw buffer handlers, see lv_ambiq_buffer.c.
 */
void lv_draw_ambiq_init_buf_handlers(void);

lv_result_t lv_draw_ambiq_nema_context_lock(void);
lv_result_t lv_draw_ambiq_nema_context_unlock(void);


nema_tex_format_t lv_ambiq_color_format_map_src(lv_color_format_t lvgl_cf);
nema_tex_format_t lv_ambiq_color_format_map_des(lv_color_format_t lvgl_cf);
uint32_t lv_ambiq_color_convert(lv_color_t color, lv_opa_t opa);

void lv_ambiq_set_blend_blit(lv_draw_ambiq_unit_t* unit, uint32_t blending_mode);
void lv_ambiq_set_blend_fill(lv_draw_ambiq_unit_t* unit, uint32_t blending_mode);
void lv_ambiq_change_blend_mode(lv_draw_ambiq_unit_t* unit, uint32_t blending_mode, 
                                 nema_tex_t dst_tex, nema_tex_t fg_tex, nema_tex_t bg_tex, bool force);
void lv_ambiq_clear_blend_mode(lv_draw_ambiq_unit_t* unit);
lv_draw_ambiq_unit_t * lv_draw_ambiq_get_default_unit(void);

/***********************
 * GLOBAL VARIABLES
 ***********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_AMBIQ*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_AMBIQ_PRIVATE_H*/
