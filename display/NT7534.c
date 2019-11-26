/**
 * @file NT7534.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "NT7534.h"
#if USE_NT7534

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "lvgl/src/lv_hal/lv_hal_disp.h"
#include <stdio.h>
#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE

/*********************
 *      DEFINES
 *********************/
#define NT7534_BAUD      2000000    /*< 2,5 MHz (400 ns)*/

#define NT7534_CMD_MODE  0
#define NT7534_DATA_MODE 1

#define NT7534_HOR_RES  128
#define NT7534_VER_RES  64

#define CMD_DISPLAY_OFF         0xAE
#define CMD_DISPLAY_ON          0xAF

#define CMD_SET_DISP_START_LINE 0x40
#define CMD_SET_PAGE            0xB0

#define CMD_SET_COLUMN_UPPER    0x10
#define CMD_SET_COLUMN_LOWER    0x00

#define CMD_SET_ADC_NORMAL      0xA0
#define CMD_SET_ADC_REVERSE     0xA1

#define CMD_SET_DISP_NORMAL     0xA6
#define CMD_SET_DISP_REVERSE    0xA7

#define CMD_SET_ALLPTS_NORMAL   0xA4
#define CMD_SET_ALLPTS_ON       0xA5
#define CMD_SET_BIAS_9          0xA2
#define CMD_SET_BIAS_7          0xA3

#define CMD_RMW                 0xE0
#define CMD_RMW_CLEAR           0xEE
#define CMD_INTERNAL_RESET      0xE2
#define CMD_SET_COM_NORMAL      0xC0
#define CMD_SET_COM_REVERSE     0xC8
#define CMD_SET_POWER_CONTROL   0x28
#define CMD_SET_RESISTOR_RATIO  0x20
#define CMD_SET_VOLUME_FIRST    0x81
#define CMD_SET_VOLUME_SECOND   0x00
#define CMD_SET_STATIC_OFF      0xAC
#define CMD_SET_STATIC_ON       0xAD
#define CMD_SET_STATIC_REG      0x00
#define CMD_SET_BOOSTER_FIRST   0xF8
#define CMD_SET_BOOSTER_234     0x00
#define CMD_SET_BOOSTER_5       0x01
#define CMD_SET_BOOSTER_6       0x03
#define CMD_NOP                 0xE3
#define CMD_TEST                0xF0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void nt7534_sync(uint8_t *data, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2);
static void nt7534_command(uint8_t cmd);
static void nt7534_data(uint8_t data);

/**********************
 *  STATIC VARIABLES
 **********************/
static uint8_t pagemap[] = { 7, 6, 5, 4, 3, 2, 1, 0 };

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the NT7534
 */
void nt7534_init(void)
{
    LV_DRV_DISP_RST(1);
    LV_DRV_DELAY_MS(10);
    LV_DRV_DISP_RST(0);
    LV_DRV_DELAY_MS(10);
    LV_DRV_DISP_RST(1);
    LV_DRV_DELAY_MS(10);

    LV_DRV_DISP_SPI_CS(0);

    nt7534_command(0xe2);
    nt7534_command(0xa2);
    nt7534_command(0xa0);
    nt7534_command(0xc0);
    nt7534_command(0x40);
    nt7534_command(0x28|0x4);
    nt7534_command(0x28|0x6);
    nt7534_command(0x28|0x7);
    nt7534_command(0x10);
    nt7534_command(0xa7);
    nt7534_command(0x81);
    nt7534_command(0x1e);
    nt7534_command(0xaf);

    /*nt7534_command(CMD_SET_BIAS_7);
    nt7534_command(CMD_SET_ADC_NORMAL);
    nt7534_command(CMD_SET_COM_NORMAL);
    nt7534_command(CMD_SET_DISP_START_LINE);
    nt7534_command(CMD_SET_POWER_CONTROL | 0x4);
    LV_DRV_DELAY_MS(50);

    nt7534_command(CMD_SET_POWER_CONTROL | 0x6);
    LV_DRV_DELAY_MS(50);

    nt7534_command(CMD_SET_POWER_CONTROL | 0x7);
    LV_DRV_DELAY_MS(10);

    nt7534_command(CMD_SET_RESISTOR_RATIO | 0x6); // Defaulted to 0x26 (but could also be between 0x20-0x27 based on display's specs)

    nt7534_command(CMD_DISPLAY_ON);
    nt7534_command(CMD_SET_ALLPTS_NORMAL);*/

    /*Set brightness*/
    //nt7534_command(CMD_SET_VOLUME_FIRST);
    nt7534_command(CMD_SET_VOLUME_SECOND | (0x18 & 0x3f));

    LV_DRV_DISP_SPI_CS(1);
}

void nt7534_set_px(struct _disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    int col = x;
    int row = y/8;
    buf += buf_w * row;
    buf += col;
    //buf += buf_w/8 * y;
    //buf += x/8;
    if(lv_color_brightness(color) > 128) {(*buf) |= (1 << (7 - y % 8));}
    else {(*buf) &= ~(1 << (7 - y % 8));}
}


void nt7534_rounder(struct _disp_drv_t * disp_drv, lv_area_t *a)
{
    a->y1 = a->y1 & ~(0x7);
    a->y2 = a->y2 |  (0x7);
    a->x2 = a->x2 > NT7534_HOR_RES-1 ? NT7534_HOR_RES-1 : a->x2;
    a->y2 = a->y2 > NT7534_VER_RES-1 ? NT7534_VER_RES-1 : a->y2;
}

void nt7534_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    printf("Flushing area\n");
    /*Return if the area is out the screen*/
    if(area->x2 < 0) return;
    if(area->y2 < 0) return;
    if(area->x1 > NT7534_HOR_RES - 1) return;
    if(area->y1 > NT7534_VER_RES - 1) return;

    /*Truncate the area to the screen*/
    lv_coord_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    lv_coord_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    lv_coord_t act_x2 = area->x2 > NT7534_HOR_RES - 1 ? NT7534_HOR_RES - 1 : area->x2;
    lv_coord_t act_y2 = area->y2 > NT7534_VER_RES - 1 ? NT7534_VER_RES - 1 : area->y2;

    //lv_coord_t x, y;
    uint8_t *buffer = (uint8_t *) color_p;

    /*Refresh frame buffer*/
    nt7534_sync(buffer, act_x1, act_y1, act_x2, act_y2);
    lv_disp_flush_ready(disp_drv);
}


/*
void nt7534_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
    if(x2 < 0) return;
    if(y2 < 0) return;
    if(x1 > NT7534_HOR_RES - 1) return;
    if(y1 > NT7534_VER_RES - 1) return;

    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > NT7534_HOR_RES - 1 ? NT7534_HOR_RES - 1 : x2;
    int32_t act_y2 = y2 > NT7534_VER_RES - 1 ? NT7534_VER_RES - 1 : y2;

    int32_t x, y;
    uint8_t white = lv_color_to1(color);

    for(y = act_y1; y <= act_y2; y++) {
        for(x = act_x1; x <= act_x2; x++) {
            if(white != 0) {
                lcd_fb[x + (y / 8)*NT7534_HOR_RES] |= (1 << (7 - (y % 8)));
            } else {
                lcd_fb[x + (y / 8)*NT7534_HOR_RES] &= ~(1 << (7 - (y % 8)));
            }
        }
    }

    //nt7534_sync(act_x1, act_y1, act_x2, act_y2);
}
*/

/**********************
 *   STATIC FUNCTIONS
 **********************/
/**
 * Flush a specific part of the buffer to the display
 * @param x1 left coordinate of the area to flush
 * @param y1 top coordinate of the area to flush
 * @param x2 right coordinate of the area to flush
 * @param y2 bottom coordinate of the area to flush
 */
static void nt7534_sync(uint8_t *data, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2)
{
    printf("Syncing area\n");

    LV_DRV_DISP_SPI_CS(0);

    uint8_t c, p;
    for(p = y1 / 8; p <= y2 / 8; p++) {
        nt7534_command(CMD_SET_PAGE | pagemap[p]);
        nt7534_command(CMD_SET_COLUMN_LOWER | (x1 & 0xf));
        nt7534_command(CMD_SET_COLUMN_UPPER | ((x1 >> 4) & 0xf));
        nt7534_command(CMD_RMW);

        for(c = x1; c <= x2; c++) {
            int row = p - y1/8;
            nt7534_data(data[(NT7534_HOR_RES * row) + c]);
        }
    }

    LV_DRV_DISP_SPI_CS(1);
}

/**
 * Write a command to the NT7534
 * @param cmd the command
 */
static void nt7534_command(uint8_t cmd)
{
    LV_DRV_DISP_CMD_DATA(NT7534_CMD_MODE);
    LV_DRV_DISP_SPI_WR_BYTE(cmd);
}

/**
 * Write data to the NT7534
 * @param data the data
 */
static void nt7534_data(uint8_t data)
{
    LV_DRV_DISP_CMD_DATA(NT7534_DATA_MODE);
    LV_DRV_DISP_SPI_WR_BYTE(data);
}

#endif
