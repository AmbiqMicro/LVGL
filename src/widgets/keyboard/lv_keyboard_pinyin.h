/**
 * @file lv_keyboard_pinyin.h
 *       reference https://github.com/xiaohua4758/lvgl8.0.1_cn_keyboard
 *
 */

#ifndef LV_KEYBOARD_PINYIN_H
#define LV_KEYBOARD_PINYIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"

#if (LV_KEYBOARD_PINYIN && LV_KEYBOARD_PINYIN_USE_DEFAULT_DICT)

#include <stddef.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    char * pinyin_list;
    const char * pinyin_mb_list;
} lv_keyboard_pinyin_dict_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

extern const lv_keyboard_pinyin_dict_t * lv_def_pinyin_dict[];

/*=====================
 * Setter functions
 *====================*/

/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif  /*LV_KEYBOARD_PINYIN_USE_DEFAULT_DICT*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_KEYBOARD_PINYIN_H*/
