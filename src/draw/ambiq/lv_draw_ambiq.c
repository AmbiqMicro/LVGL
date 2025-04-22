/**
 * @file lv_draw_ambiq.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw_private.h"
#if LV_USE_DRAW_AMBIQ

#include "../../core/lv_refr.h"
#include "lv_draw_ambiq.h"
#include "lv_draw_ambiq_private.h"
#include "../../display/lv_display_private.h"
#include "../../stdlib/lv_string.h"
#include "../../core/lv_global.h"

/*********************
 *      DEFINES
 *********************/
#define DRAW_UNIT_ID_AMBIQ     9

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if LV_USE_OS
    static void render_thread_cb(void * ptr);
#endif

static void execute_drawing(lv_draw_task_t * t);

static int32_t dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer);
static int32_t evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task);
static int32_t lv_draw_ambiq_delete(lv_draw_unit_t * draw_unit);

/**********************
 *  STATIC VARIABLES
 **********************/
#define _draw_info LV_GLOBAL_DEFAULT()->draw_info
static lv_draw_ambiq_unit_t * draw_ambiq_unit = NULL;


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_ambiq_init(void)
{
    lv_draw_ambiq_init_buf_handlers();

    draw_ambiq_unit = lv_draw_create_unit(sizeof(lv_draw_ambiq_unit_t));
    draw_ambiq_unit->base_unit.dispatch_cb = dispatch;
    draw_ambiq_unit->base_unit.evaluate_cb = evaluate;
    draw_ambiq_unit->base_unit.delete_cb = LV_USE_OS ? lv_draw_ambiq_delete : NULL;
    draw_ambiq_unit->base_unit.name = "AMBIQ";
    draw_ambiq_unit->cl_head = NULL;
    draw_ambiq_unit->total_buffer = 0;
    draw_ambiq_unit->total_cl = 0;
    draw_ambiq_unit->small_texture_buffer_size_byte = 256;
    draw_ambiq_unit->small_texture_buffer  = nema_buffer_create_pool(NEMA_MEM_POOL_FB, draw_ambiq_unit->small_texture_buffer_size_byte);
#if LV_USE_VECTOR_GRAPHIC
    draw_ambiq_unit->vg_path = nema_vg_path_create();
    draw_ambiq_unit->vg_paint = nema_vg_paint_create();
    draw_ambiq_unit->vg_grad = nema_vg_grad_create();
#endif

    draw_ambiq_unit->blend_mode = 0;
    draw_ambiq_unit->dst_tex = NEMA_NOTEX;
    draw_ambiq_unit->fg_tex = NEMA_NOTEX;
    draw_ambiq_unit->bg_tex = NEMA_NOTEX;

//    lv_ll_init(&draw_ambiq_unit->inserted_cl_ll, sizeof(nema_cmdlist_t));

    draw_ambiq_unit->nema_context_lock_count = 0;

#if LV_USE_OS
    lv_mutex_init(&draw_ambiq_unit->mutex_nema_context);
    lv_thread_init(&draw_ambiq_unit->thread, "ambiqdraw", LV_THREAD_PRIO_HIGH, render_thread_cb, LV_DRAW_THREAD_STACK_SIZE, draw_ambiq_unit);
#endif

//     // lv_gpu_ambiq_version_t gpu_lib_ver = {0};
//     // uint32_t gpu_ret = _lv_gpu_ambiq_nema_init(&gpu_lib_ver);

//     // if((gpu_ret != NEMA_ERR_NO_ERROR) && (gpu_ret != NEMA_ERR_NO_INIT))
//     // {
//     //     LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
//     //     return ;
//     // }
//     // LV_LOG_INFO("GPU Lib Version: %d.%d.%d\n", gpu_lib_ver.s.Major, gpu_lib_ver.s.Minor, gpu_lib_ver.s.Revision);

// #if defined(NEMA_GFX_POWERSAVE) && defined(NEMA_GFX_POWEROFF_END_CL)
//     nemagfx_set_interrupt_callback(lv_ambiq_nema_gpu_isr_cb);
// #endif

// #ifndef NEMA_GFX_POWERSAVE
//     //Power on GPU
//     lv_result_t ret = lv_ambiq_nema_gpu_power_on();
//     if (ret != LV_RESULT_OK)
//     {
//         LV_LOG_ERROR("Ambiq GPU init failed!\n");
//         return ;
//     }
// #endif
}

void lv_draw_ambiq_deinit(void)
{
    // lv_result_t ret;
    // ret = lv_ambiq_nema_gpu_check_busy_and_suspend();
    // if(ret == LV_RESULT_OK)
    // {
    //     LV_LOG_ERROR("GPU is still busy, cannot poweroff now!\n");
    //     return ;   
    // }

#if LV_USE_DRAW_AMBIQ_VG
    //This will release the internal buffer in NemaVG.
    nema_vg_deinit();
#endif

    // Call low level API to release global ring buffer
}

static int32_t lv_draw_ambiq_delete(lv_draw_unit_t * draw_unit)
{
    lv_draw_ambiq_unit_t * draw_ambiq_unit = (lv_draw_ambiq_unit_t *) draw_unit;

    //clean command list buffer
    if(draw_ambiq_unit->cl_head != NULL)
    {
        nema_cl_destroy(draw_ambiq_unit->cl_head);
    }

    nema_buffer_destroy(&draw_ambiq_unit->small_texture_buffer);

#if LV_USE_VECTOR_GRAPHIC
    //Release VG path
    nema_vg_path_destroy(draw_ambiq_unit->vg_path);

    //Release VG paint
    nema_vg_paint_destroy(draw_ambiq_unit->vg_paint);

    //Release VG gradient
    nema_vg_grad_destroy(draw_ambiq_unit->vg_grad);
#endif

    

#if LV_USE_OS
    LV_LOG_INFO("cancel Ambiq GPU rendering thread");
    draw_ambiq_unit->exit_status = true;

    if(draw_ambiq_unit->inited) {
        lv_thread_sync_signal(&draw_ambiq_unit->sync);
    }

    return lv_thread_delete(&draw_ambiq_unit->thread);
#else
    LV_UNUSED(draw_unit);
    return 0;
#endif
}



/**********************
 *   STATIC FUNCTIONS
 **********************/
static inline void execute_drawing_unit(lv_draw_task_t * t)
{
    execute_drawing(t);

    lv_draw_ambiq_unit_t * draw_ambiq_unit = (lv_draw_ambiq_unit_t *)t->draw_unit;

    t->state = LV_DRAW_TASK_STATE_READY;
    draw_ambiq_unit->task_act = NULL;

    /*The draw unit is free now. Request a new dispatching as it can get a new task*/
    lv_draw_dispatch_request();
}

static int32_t evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task)
{
    LV_UNUSED(draw_unit);

    switch(task->type) {
        case LV_DRAW_TASK_TYPE_FILL:
            lv_draw_fill_dsc_t * draw_dsc_fill = task->draw_dsc;

            if(draw_dsc_fill->grad.dir == LV_GRAD_DIR_RADIAL ||
                draw_dsc_fill->grad.dir == LV_GRAD_DIR_CONICAL ||
                draw_dsc_fill->grad.dir == LV_GRAD_DIR_LINEAR)
            {
                /**
                 * Note: The underlying system supports these features; however, 
                 * their full implementation and support are currently under development.
                 */
                return 0;
            }

            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;
        case LV_DRAW_TASK_TYPE_BORDER:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;
        case LV_DRAW_TASK_TYPE_BOX_SHADOW:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;
        case LV_DRAW_TASK_TYPE_TRIANGLE:
            //return 0;
            lv_draw_triangle_dsc_t * draw_dsc_tri = task->draw_dsc;

            if(draw_dsc_tri->grad.dir == LV_GRAD_DIR_RADIAL ||
                draw_dsc_tri->grad.dir == LV_GRAD_DIR_CONICAL ||
                draw_dsc_tri->grad.dir == LV_GRAD_DIR_LINEAR)
            {
                /**
                 * Note: The underlying system supports these features; however, 
                 * their full implementation and support are currently under development.
                 */
                return 0;
            }
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;              
        case LV_DRAW_TASK_TYPE_LINE:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break; 

        case LV_DRAW_TASK_TYPE_ARC:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;

        case LV_DRAW_TASK_TYPE_LAYER:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;

        case LV_DRAW_TASK_TYPE_IMAGE:
            lv_draw_image_dsc_t * draw_dsc_image = task->draw_dsc;

            nema_tex_format_t nema_cf = lv_ambiq_color_format_map_src(draw_dsc_image->header.cf);
            if(nema_cf == COLOR_FORMAT_INVALID)
            {
                return 0;
            }

            //Set blend mode
            if((draw_dsc_image->blend_mode == LV_BLEND_MODE_SUBTRACTIVE) || 
                (draw_dsc_image->blend_mode == LV_BLEND_MODE_MULTIPLY))
            {
                return 0;  
            }

            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;          
            break;

        case LV_DRAW_TASK_TYPE_LABEL:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;

        case LV_DRAW_TASK_TYPE_MASK_RECTANGLE:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;

        case LV_DRAW_TASK_TYPE_VECTOR:
            task->preference_score = 10;
            task->preferred_draw_unit_id = DRAW_UNIT_ID_AMBIQ;
            break;
        default:
            break;
    }

    return 0;
}

static int32_t dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer)
{
    LV_PROFILER_DRAW_BEGIN;
    lv_draw_ambiq_unit_t * draw_ambiq_unit = (lv_draw_ambiq_unit_t *) draw_unit;

    /*Return immediately if it's busy with draw task*/
    if(draw_ambiq_unit->task_act) {
        LV_PROFILER_DRAW_END;
        return 0;
    }

    lv_draw_task_t * t = NULL;
    t = lv_draw_get_available_task(layer, NULL, DRAW_UNIT_ID_AMBIQ);
    if(t == NULL) {
        LV_PROFILER_DRAW_END;
        return LV_DRAW_UNIT_IDLE;
    }

    void * buf = lv_draw_layer_alloc_buf(layer);
    if(buf == NULL) {
        LV_PROFILER_DRAW_END;
        return LV_DRAW_UNIT_IDLE;
    }

    t->state = LV_DRAW_TASK_STATE_IN_PROGRESS;
    t->draw_unit = (lv_draw_unit_t *)draw_unit;
    draw_ambiq_unit->task_act = t;

#if LV_USE_OS
    /*Let the render thread work*/
    if(draw_ambiq_unit->inited) lv_thread_sync_signal(&draw_ambiq_unit->sync);
#else
    execute_drawing_unit(t);
#endif
    LV_PROFILER_DRAW_END;
    return 1;
}

#if LV_USE_OS
static void render_thread_cb(void * ptr)
{
    lv_draw_ambiq_unit_t * u = ptr;

    lv_thread_sync_init(&u->sync);
    u->inited = true;

    while(1) {
        while(u->task_act == NULL && !u->exit_status) {
            lv_thread_sync_wait(&u->sync);
        }

        if(u->exit_status) {
            LV_LOG_INFO("ready to exit software rendering thread");
            break;
        }

        // if(!lv_ll_is_empty(&u->inserted_cl_ll))
        // {
        //     lv_mutex_lock(&u->mutex_inserted_cl_ll);
        //     nema_cmdlist_t * cl = lv_ll_get_head(&u->inserted_cl_ll);
        //     while(cl)
        //     {

        //         lv_ll_remove(&u->inserted_cl_ll, cl);
        //         lv_mutex_unlock(&u->mutex_inserted_cl_ll);

        //         nema_cl_submit(cl);
        //         nema_cl_wait(cl);
        //         nema_cl_destroy(cl);
        //         lv_free(cl);


        //         lv_mutex_lock(&u->mutex_inserted_cl_ll);
        //         cl = lv_ll_get_head(&u->inserted_cl_ll);
        //     }

        //     lv_mutex_unlock(&u->mutex_inserted_cl_ll);
        // }

        execute_drawing_unit(u->task_act);
    }

    u->inited = false;
    lv_thread_sync_delete(&u->sync);
    lv_mutex_delete(&u->mutex_nema_context);
    LV_LOG_INFO("exit software rendering thread");
}
#endif

static void execute_drawing(lv_draw_task_t * t)
{
    LV_PROFILER_DRAW_BEGIN;

    /*Render the draw task*/
    lv_layer_t * layer = t->target_layer;
    lv_draw_buf_t * draw_buf = layer->draw_buf;

    lv_area_t clip_area;
    lv_area_copy(&clip_area, &t->clip_area);
    lv_area_move(&clip_area, -layer->buf_area.x1, -layer->buf_area.y1);

    lv_area_t draw_area;
    lv_area_copy(&draw_area, &t->area);
    lv_area_move(&draw_area, -layer->buf_area.x1, -layer->buf_area.y1);

    if(!lv_area_intersect(&draw_area, &draw_area, &clip_area))
        return; /*Fully clipped, nothing to do*/

    /* Invalidate the drawing area */
    lv_draw_buf_invalidate_cache(draw_buf, &draw_area);

    nema_cmdlist_t cl = nema_cl_create_sized(10*1024);

    lv_draw_ambiq_nema_context_lock();

    nema_cl_bind_circular(&cl);

    nema_cl_rewind(&cl);

    nema_tex_format_t des_format = lv_ambiq_color_format_map_des(draw_buf->header.cf);
    if(des_format == COLOR_FORMAT_INVALID)
    {
        LV_LOG_ERROR("Unsupported layer color format!");
        nema_cl_destroy(&cl);
        return;
    }

    /* Set target buffer */
    nema_bind_tex(NEMA_TEX0, 
                 (uintptr_t)draw_buf->data, 
                 draw_buf->header.w, draw_buf->header.h, 
                 des_format, draw_buf->header.stride,
                 0);

    /* Set clip area*/
    nema_set_clip(clip_area.x1, clip_area.y1, lv_area_get_width(&clip_area), lv_area_get_height(&clip_area));

    /* Clear internal blend mode status*/
    lv_ambiq_clear_blend_mode(NULL);

    switch(t->type) {
        case LV_DRAW_TASK_TYPE_FILL:
            lv_draw_ambiq_fill(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_BORDER:
            lv_draw_ambiq_border(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_TRIANGLE:
            lv_draw_ambiq_triangle(t, t->draw_dsc);
            break;
        case LV_DRAW_TASK_TYPE_LINE:
            lv_draw_ambiq_line(t, t->draw_dsc);
            break;  
        case LV_DRAW_TASK_TYPE_ARC:
            lv_draw_ambiq_arc(t, t->draw_dsc, &t->area);
            break; 
        case LV_DRAW_TASK_TYPE_IMAGE:
            lv_draw_ambiq_image(t, t->draw_dsc, &t->area);
            break; 
        case LV_DRAW_TASK_TYPE_LABEL:
            lv_draw_ambiq_label(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_LETTER:
            //Under development.
            //lv_draw_ambiq_letter(t, t->draw_dsc, &t->area);
            break;          
        case LV_DRAW_TASK_TYPE_BOX_SHADOW:
            lv_draw_ambiq_box_shadow(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_MASK_RECTANGLE:
            lv_draw_ambiq_mask_rect(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_LAYER:
            lv_draw_ambiq_layer(t, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_VECTOR:
            lv_draw_ambiq_vector(t, t->draw_dsc);
            break;

        default:
            break;
    }

    nema_cl_submit(&cl);

    lv_draw_ambiq_nema_context_unlock();

    nema_cl_wait(&cl);

    uint32_t nema_error_code = nema_get_error();
    if(nema_error_code != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("NemaVG error: 0x%08lX", nema_error_code);
    }

    uint32_t nema_vg_error_code = nema_vg_get_error();
    if(nema_vg_error_code != NEMA_VG_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("NemaVG error: 0x%08lX", nema_vg_error_code);
    }

    nema_cl_destroy(&cl);


    LV_PROFILER_DRAW_END;
}

lv_result_t lv_draw_ambiq_nema_context_lock(void)
{
#if LV_USE_OS
    LV_ASSERT_MSG(draw_ambiq_unit != NULL, "Ambiq GPU draw unit is not initialized!");
    lv_result_t res = lv_mutex_lock(&draw_ambiq_unit->mutex_nema_context);
    if(res != LV_RESULT_OK)
    {
        LV_LOG_ERROR("Ambiq GPU mutex lock failed!");
    }
    else
    {
        draw_ambiq_unit->nema_context_lock_count ++;
        LV_ASSERT_MSG(draw_ambiq_unit->nema_context_lock_count == 1, "Ambiq GPU mutex lock count error!");
    }
    return res;
#else
    draw_ambiq_unit->nema_context_lock_count ++;
    LV_ASSERT_MSG(draw_ambiq_unit->nema_context_lock_count == 1, "Ambiq GPU mutex lock count error!");
    return LV_RESULT_OK;
#endif
}

lv_result_t lv_draw_ambiq_nema_context_unlock(void)
{
#if LV_USE_OS
    LV_ASSERT_MSG(draw_ambiq_unit != NULL, "Ambiq GPU draw unit is not initialized!");
    lv_result_t res = lv_mutex_unlock(&draw_ambiq_unit->mutex_nema_context);
    if(res != LV_RESULT_OK)
    {
        LV_LOG_ERROR("Ambiq GPU mutex unlock failed!");
    }
    else
    {
        draw_ambiq_unit->nema_context_lock_count --;
        LV_ASSERT_MSG(draw_ambiq_unit->nema_context_lock_count == 0, "Ambiq GPU mutex lock count error!");
    }
    return res;
#else
    draw_ambiq_unit->nema_context_lock_count --;
    LV_ASSERT_MSG(draw_ambiq_unit->nema_context_lock_count == 0, "Ambiq GPU mutex lock count error!");    
    return LV_RESULT_OK;
#endif
}

lv_draw_ambiq_unit_t * lv_draw_ambiq_get_default_unit(void)
{
    return draw_ambiq_unit;
}

// lv_result_t lv_draw_ambiq_nema_buffer_lock(void)
// {
// #if LV_USE_OS
//     LV_ASSERT_MSG(draw_ambiq_unit != NULL, "Ambiq GPU draw unit is not initialized!");
//     lv_result_t res = lv_mutex_lock(&draw_ambiq_unit->mutex_buffer_create);
//     if(res != LV_RESULT_OK)
//     {
//         LV_LOG_ERROR("Ambiq GPU mutex lock failed!");
//     }
//     return res;
// #else
//     return LV_RESULT_OK;
// #endif
// }

// lv_result_t lv_draw_ambiq_nema_buffer_unlock(void)
// {
// #if LV_USE_OS
//     LV_ASSERT_MSG(draw_ambiq_unit != NULL, "Ambiq GPU draw unit is not initialized!");
//     lv_result_t res = lv_mutex_unlock(&draw_ambiq_unit->mutex_buffer_create);
//     if(res != LV_RESULT_OK)
//     {
//         LV_LOG_ERROR("Ambiq GPU mutex unlock failed!");
//     }
//     return res;
// #else
//     return LV_RESULT_OK;
// #endif
// }

#endif /*LV_USE_DRAW_AMBIQ*/
