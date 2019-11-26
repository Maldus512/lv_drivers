/**
 * @file ST7565.h
 *
 */

#ifndef ST7565_H
#define ST7565_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_NT7534

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void nt7534_init(void);
void nt7534_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
void nt7534_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);
void nt7534_set_px(struct _disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);
void nt7534_rounder(struct _disp_drv_t * disp_drv, lv_area_t *a);

/**********************
 *      MACROS
 **********************/

#endif /* USE_ST7565 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ST7565_H */
