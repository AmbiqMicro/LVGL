/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 0

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
    #define MY_DISP_HOR_RES    320
#endif

#ifndef MY_DISP_VER_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
    #define MY_DISP_VER_RES    240
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

#define DEF_STATIC_DRAW_BUF(__NAME, __WIDTH, __HEIGHT, __FORAMT)   \
    static uint8_t __NAME##_array[_LV_DRAW_BUF_SIZE((__WIDTH), (__HEIGHT), __FORAMT)];  \
    static lv_draw_buf_t __NAME;   \
    lv_draw_buf_init(&__NAME, (__WIDTH), (__HEIGHT), __FORAMT, 0, __NAME##_array, sizeof(__NAME##_array));

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);

    /* Example 1
     * One buffer for partial rendering*/
    DEF_STATIC_DRAW_BUF(buf_1_1, MY_DISP_HOR_RES, MY_DISP_VER_RES / 10, LV_COLOR_FORMAT_NATIVE); /*A buffer for 1/10 fb*/

    lv_display_set_draw_buffers(disp, &buf_1_1, NULL);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Example 2
     * Two buffers for partial rendering
     * In flush_cb DMA or similar hardware should be used to update the display in the background.*/
    DEF_STATIC_DRAW_BUF(buf_2_1, MY_DISP_HOR_RES, MY_DISP_VER_RES / 10, LV_COLOR_FORMAT_NATIVE); /*A buffer for 1/10 fb*/
    DEF_STATIC_DRAW_BUF(buf_2_2, MY_DISP_HOR_RES, MY_DISP_VER_RES / 10, LV_COLOR_FORMAT_NATIVE); /*A buffer for 1/10 fb*/
    lv_display_set_draw_buffers(disp, &buf_2_1, &buf_2_2);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Example 3
     * Two buffers screen sized buffer for double buffering.
     * Both LV_DISPLAY_RENDER_MODE_DIRECT and LV_DISPLAY_RENDER_MODE_FULL works, see their comments*/
    DEF_STATIC_DRAW_BUF(buf_3_1, MY_DISP_HOR_RES, MY_DISP_VER_RES, LV_COLOR_FORMAT_NATIVE);
    DEF_STATIC_DRAW_BUF(buf_3_2, MY_DISP_HOR_RES, MY_DISP_VER_RES, LV_COLOR_FORMAT_NATIVE);
    lv_display_set_draw_buffers(disp, &buf_3_1, &buf_3_2);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_DIRECT);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
static void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map)
{
    if(disp_flush_enabled) {
        /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

        int32_t x;
        int32_t y;
        for(y = area->y1; y <= area->y2; y++) {
            for(x = area->x1; x <= area->x2; x++) {
                /*Put a pixel to the display. For example:*/
                /*put_px(x, y, *px_map)*/
                px_map++;
            }
        }
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
