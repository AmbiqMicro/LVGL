/**
 * @file lv_st7789.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_st7789.h"

/*********************
 *      DEFINES
 *********************/

#define CMD_FRMCTR1		0xB1
#define CMD_FRMCTR2		0xB2
#define CMD_FRMCTR3		0xB3
#define CMD_INVCTR		0xB4
#define CMD_DFUNCTR		0xB6
#define CMD_ETMOD		0xB7
#define CMD_PWCTR1		0xC0
#define CMD_PWCTR2		0xC1
#define CMD_PWCTR3		0xC2
#define CMD_PWCTR4		0xC3
#define CMD_PWCTR5		0xC4
#define CMD_VMCTR		0xC5
#define CMD_GMCTRP1		0xE0
#define CMD_GMCTRN1		0xE1
#define CMD_DOCA		0xE8
#define CMD_CSCON		0xF0

#define CMD_RAMCTRL		0xB0
#define CMD_PORCTRL		0xB2	/* Porch control */
#define CMD_GCTRL		0xB7	/* Gate control */
#define CMD_VCOMS		0xBB	/* VCOMS setting */
#define CMD_LCMCTRL		0xC0	/* LCM control */
#define CMD_VDVVRHEN	0xC2	/* VDV and VRH command enable */
#define CMD_VRHS		0xC3	/* VRH set */
#define CMD_VDVSET		0xC4	/* VDV setting */
#define CMD_FRCTR2		0xC6	/* FR Control 2 */
#define CMD_PWCTRL1		0xD0	/* Power control 1 */
#define CMD_PVGAMCTRL	0xE0	/* Positive voltage gamma control */
#define CMD_NVGAMCTRL	0xE1	/* Negative voltage gamma control */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void send_cmd(lv_lcd_generic_mipi_driver_t *drv, uint8_t cmd, uint8_t *param, size_t param_size);
static lv_lcd_generic_mipi_driver_t *get_driver(lv_display_t *disp);

/**********************
 *  STATIC CONSTANTS
 **********************/

// init commands based on LovyanGFX
static const uint8_t init_cmd_list[] = {
	CMD_PORCTRL, 5, 0x0c, 0x0c, 0x00, 0x33, 0x33,
	CMD_GCTRL  , 1, 0x35,

	CMD_VCOMS  , 1, 0x28,		/* JLX240 display datasheet */
	CMD_LCMCTRL, 1, 0x0C,
	CMD_VDVVRHEN,2, 0x01, 0xFF,
	CMD_VRHS   , 1, 0x10,		/* voltage VRHS */
	CMD_VDVSET , 1, 0x20,
	CMD_FRCTR2 , 1, 0x0f,		/* 0x0f = 60Hz */
	CMD_PWCTRL1, 2, 0xa4, 0xa1,
	CMD_RAMCTRL, 2, 0x00, 0xC0,
	CMD_PVGAMCTRL, 14,
					0xd0, 0x00, 0x02, 0x07,
					0x0a, 0x28, 0x32, 0x44,
					0x42, 0x06, 0x0e, 0x12,
					0x14, 0x17,
	CMD_NVGAMCTRL, 14,
					0xd0, 0x00, 0x02, 0x07,
					0x0a, 0x28, 0x31, 0x54,
					0x47, 0x0e, 0x1c, 0x17,
					0x1b, 0x1e,
	LV_LCD_CMD_SPECIAL, LV_LCD_CMD_EOF
};

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_display_t *lv_st7789_create(uint32_t hor_res, uint32_t ver_res, lv_lcd_flag_t flags, lv_st7789_send_cmd_cb_t send_cmd_cb, lv_st7789_send_cmd_cb_t send_color_cb)
{
	lv_display_t *disp = lv_lcd_generic_mipi_create(hor_res, ver_res, flags, send_cmd_cb, send_color_cb);
	lv_lcd_generic_mipi_driver_t *drv = get_driver(disp);
    lv_lcd_generic_mipi_send_cmd_list(drv, init_cmd_list);
    return disp;
}

void lv_st7789_set_gap(lv_display_t *disp, uint16_t x, uint16_t y)
{
	lv_lcd_generic_mipi_set_gap(disp, x, y);
}

void lv_st7789_set_invert(lv_display_t *disp, bool invert)
{
	lv_lcd_generic_mipi_set_invert(disp, invert);
}

void lv_st7789_set_gamma_curve(lv_display_t *disp, uint8_t gamma)
{
	lv_lcd_generic_mipi_set_gamma_curve(disp, gamma);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void send_cmd(lv_lcd_generic_mipi_driver_t *drv, uint8_t cmd, uint8_t *param, size_t param_size)
{
    uint8_t cmdbuf = cmd;		/* MIPI uses 8 bit commands */
    drv->send_cmd(drv->disp, &cmdbuf, 1, param, param_size);
}

static lv_lcd_generic_mipi_driver_t *get_driver(lv_display_t *disp)
{
	return (lv_lcd_generic_mipi_driver_t *)lv_display_get_driver_data(disp);
}


