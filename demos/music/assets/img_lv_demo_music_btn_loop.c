#include "music/lv_demo_music.h"
#if LV_USE_DEMO_MUSIC  && !LV_DEMO_MUSIC_LARGE

#ifndef LV_ATTRIBUTE_MEM_ALIGN
    #define LV_ATTRIBUTE_MEM_ALIGN
#endif

const LV_ATTRIBUTE_MEM_ALIGN uint8_t img_lv_demo_music_btn_loop_map[] = {
#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8
    /*Pixel format: Alpha 8 bit, Red: 3 bit, Green: 3 bit, Blue: 2 bit*/
    0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x03, 0x4a, 0x4b, 0x4a, 0x94, 0x4a, 0xc7, 0x4a, 0xdc, 0x4a, 0xdc, 0x4a, 0xbf, 0x4a, 0x84, 0x4a, 0x34, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x4b, 0x4a, 0xd4, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xbf, 0x4a, 0x37, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x03, 0x4a, 0x8f, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xfb, 0x4a, 0x6b, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0xa0, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x7b, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x6f, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x48, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x14, 0x4a, 0xf0, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xd7, 0x4a, 0x04, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x83, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xb7, 0xff, 0xff, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x5c, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0xd7, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x6e, 0xff, 0x6e, 0xff, 0x6a, 0xff, 0xb7, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8e, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xb7, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x18, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xdb, 0xff, 0xff, 0xff, 0xb7, 0xff, 0x6e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6e, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xef, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x34, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xdb, 0xff, 0xff, 0xff, 0x92, 0xff, 0x6e, 0xff, 0x6a, 0xff, 0xb7, 0xff, 0xff, 0xff, 0xdb, 0xff, 0x92, 0xff, 0xff, 0xff, 0xff, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x08, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x44, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xff, 0xff, 0xb3, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xb7, 0xff, 0xff, 0xff, 0x4a, 0xff, 0x92, 0xff, 0xff, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x1b, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x33, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xff, 0xff, 0xb7, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xb7, 0xff, 0xff, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x08, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x0c, 0x4a, 0xf8, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xdb, 0xff, 0xff, 0xff, 0xb7, 0xff, 0x92, 0xff, 0x92, 0xff, 0x92, 0xff, 0x92, 0xff, 0x92, 0xff, 0xb7, 0xff, 0xff, 0xff, 0xdb, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xdc, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0xb8, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0xd7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdb, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x98, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x58, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0x30, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x03, 0x4a, 0xcf, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xa4, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x2c, 0x4a, 0xf3, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xe8, 0x4a, 0x18, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x4c, 0x4a, 0xf7, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xe8, 0x4a, 0x30, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x34, 0x4a, 0xe0, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xcb, 0x4a, 0x1f, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x08, 0x4a, 0x73, 0x4a, 0xe3, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xff, 0x4a, 0xd0, 0x4a, 0x5c, 0x4a, 0x03, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x03, 0x4a, 0x33, 0x4a, 0x63, 0x4a, 0x77, 0x4a, 0x77, 0x4a, 0x5b, 0x4a, 0x20, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0xff, 0x00,
    0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
    0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
#endif
#if LV_COLOR_DEPTH == 16
    /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
    0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x03, 0x6e, 0x52, 0x4b, 0x6e, 0x52, 0x94, 0x6e, 0x52, 0xc7, 0x6e, 0x52, 0xdc, 0x6e, 0x52, 0xdc, 0x6e, 0x52, 0xbf, 0x6e, 0x52, 0x84, 0x6e, 0x52, 0x34, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x4b, 0x6e, 0x52, 0xd4, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xbf, 0x6e, 0x52, 0x37, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x03, 0x6e, 0x52, 0x8f, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xfb, 0x6e, 0x52, 0x6b, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0xa0, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x7b, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x6f, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x48, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x14, 0x6e, 0x52, 0xf0, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xd7, 0x6e, 0x52, 0x04, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x83, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x57, 0xad, 0xff, 0xfc, 0xe6, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x5c, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0xd7, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x50, 0x6b, 0xff, 0x30, 0x6b, 0xff, 0x6e, 0x52, 0xff, 0xf6, 0xa4, 0xff, 0xff, 0xff, 0xff, 0x9b, 0xd6, 0xff, 0x91, 0x73, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xb7, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x18, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x5a, 0xce, 0xff, 0xff, 0xff, 0xff, 0x57, 0xad, 0xff, 0xae, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdc, 0xde, 0xff, 0xff, 0xff, 0xff, 0x1d, 0xe7, 0xff, 0xae, 0x5a, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xef, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x34, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x3a, 0xce, 0xff, 0xff, 0xff, 0xff, 0x33, 0x8c, 0xff, 0xcf, 0x5a, 0xff, 0x6e, 0x52, 0xff, 0xb5, 0x9c, 0xff, 0xff, 0xff, 0xff, 0x5a, 0xce, 0xff, 0xb2, 0x7b, 0xff, 0xff, 0xff, 0xff, 0xdc, 0xde, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x08, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x44, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0xff, 0xff, 0xff, 0x94, 0x94, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x16, 0xa5, 0xff, 0xfc, 0xe6, 0xff, 0x6e, 0x52, 0xff, 0xd2, 0x83, 0xff, 0xff, 0xff, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x1b, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x33, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0xff, 0xff, 0xff, 0xf6, 0xa4, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0xd5, 0x9c, 0xff, 0xff, 0xff, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x08, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x0c, 0x6e, 0x52, 0xf8, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0xb8, 0xb5, 0xff, 0xff, 0xff, 0xff, 0x16, 0xa5, 0xff, 0x54, 0x8c, 0xff, 0x33, 0x8c, 0xff, 0x54, 0x8c, 0xff, 0x33, 0x8c, 0xff, 0x13, 0x84, 0xff, 0x16, 0xa5, 0xff, 0xff, 0xff, 0xff, 0xd9, 0xbd, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xdc, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0xb8, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x98, 0xb5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb8, 0xbd, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x98, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x58, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0x30, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x03, 0x6e, 0x52, 0xcf, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xa4, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x2c, 0x6e, 0x52, 0xf3, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xe8, 0x6e, 0x52, 0x18, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x4c, 0x6e, 0x52, 0xf7, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xe8, 0x6e, 0x52, 0x30, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x34, 0x6e, 0x52, 0xe0, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xcb, 0x6e, 0x52, 0x1f, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x08, 0x6e, 0x52, 0x73, 0x6e, 0x52, 0xe3, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xff, 0x6e, 0x52, 0xd0, 0x6e, 0x52, 0x5c, 0x6e, 0x52, 0x03, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x03, 0x6e, 0x52, 0x33, 0x6e, 0x52, 0x63, 0x6e, 0x52, 0x77, 0x6e, 0x52, 0x77, 0x6e, 0x52, 0x5b, 0x6e, 0x52, 0x20, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0x6e, 0x52, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00,
#endif
#if LV_COLOR_DEPTH == 32
    0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x03, 0x6d, 0x4d, 0x50, 0x4b, 0x6d, 0x4d, 0x50, 0x94, 0x6d, 0x4d, 0x50, 0xc7, 0x6d, 0x4d, 0x50, 0xdc, 0x6d, 0x4d, 0x50, 0xdc, 0x6d, 0x4d, 0x50, 0xbf, 0x6d, 0x4d, 0x50, 0x84, 0x6d, 0x4d, 0x50, 0x34, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x4b, 0x6d, 0x4d, 0x50, 0xd4, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xbf, 0x6d, 0x4d, 0x50, 0x37, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x03, 0x6d, 0x4d, 0x50, 0x8f, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xfb, 0x6d, 0x4d, 0x50, 0x6b, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0xa0, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x7b, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x6f, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x48, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x14, 0x6d, 0x4d, 0x50, 0xf0, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xd7, 0x6d, 0x4d, 0x50, 0x04, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x83, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xb7, 0xa7, 0xa9, 0xff, 0xe4, 0xdd, 0xde, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x5c, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0xd7, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x84, 0x6a, 0x6c, 0xff, 0x7f, 0x63, 0x65, 0xff, 0x6e, 0x4e, 0x51, 0xff, 0xad, 0x9b, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xda, 0xd2, 0xd2, 0xff, 0x89, 0x6f, 0x72, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xb7, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x18, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xd2, 0xc9, 0xc9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb9, 0xaa, 0xab, 0xff, 0x74, 0x56, 0x59, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xd8, 0xd9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe5, 0xe0, 0xe0, 0xff, 0x72, 0x53, 0x56, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xef, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x34, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xd1, 0xc6, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9a, 0x83, 0x86, 0xff, 0x77, 0x59, 0x5c, 0xff, 0x6e, 0x4e, 0x51, 0xff, 0xa9, 0x96, 0x97, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd3, 0xc9, 0xca, 0xff, 0x8d, 0x74, 0x76, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xdf, 0xd8, 0xd9, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x08, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x44, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xff, 0xff, 0xff, 0xff, 0xa3, 0x8f, 0x91, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xb3, 0xa2, 0xa4, 0xff, 0xe4, 0xdd, 0xde, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x92, 0x7a, 0x7d, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x1b, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x33, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xff, 0xff, 0xff, 0xff, 0xaf, 0x9e, 0xa0, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xab, 0x99, 0x9b, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x08, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x0c, 0x6d, 0x4d, 0x50, 0xf8, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xc1, 0xb3, 0xb4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb3, 0xa2, 0xa4, 0xff, 0x9d, 0x87, 0x89, 0xff, 0x9c, 0x86, 0x88, 0xff, 0x9d, 0x87, 0x89, 0xff, 0x9a, 0x83, 0x86, 0xff, 0x96, 0x7f, 0x81, 0xff, 0xb1, 0x9f, 0xa1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc6, 0xb9, 0xba, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xdc, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0xb8, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0xbe, 0xb0, 0xb1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc2, 0xb4, 0xb6, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x98, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x58, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0x30, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x03, 0x6d, 0x4d, 0x50, 0xcf, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xa4, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x2c, 0x6d, 0x4d, 0x50, 0xf3, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xe8, 0x6d, 0x4d, 0x50, 0x18, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x4c, 0x6d, 0x4d, 0x50, 0xf7, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xe8, 0x6d, 0x4d, 0x50, 0x30, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x34, 0x6d, 0x4d, 0x50, 0xe0, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xcb, 0x6d, 0x4d, 0x50, 0x1f, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x08, 0x6d, 0x4d, 0x50, 0x73, 0x6d, 0x4d, 0x50, 0xe3, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xff, 0x6d, 0x4d, 0x50, 0xd0, 0x6d, 0x4d, 0x50, 0x5c, 0x6d, 0x4d, 0x50, 0x03, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x03, 0x6d, 0x4d, 0x50, 0x33, 0x6d, 0x4d, 0x50, 0x63, 0x6d, 0x4d, 0x50, 0x77, 0x6d, 0x4d, 0x50, 0x77, 0x6d, 0x4d, 0x50, 0x5b, 0x6d, 0x4d, 0x50, 0x20, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0x6d, 0x4d, 0x50, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
#endif
};

lv_image_dsc_t img_lv_demo_music_btn_loop = {
    .header.always_zero = 0,
    .header.w = 24,
    .header.h = 24,
    //    .data_size = 576 * LV_COLOR_FORMAT_NATIVE_ALPHA_SIZE,
    .header.cf = LV_COLOR_FORMAT_ARGB8888,
    .data = img_lv_demo_music_btn_loop_map,
};

#endif /*LV_USE_DEMO_MUSIC*/


