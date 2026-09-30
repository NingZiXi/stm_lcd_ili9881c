/** @file example.h @brief DSI 模组接入示例，时序和电源由板级管理。 */
#ifndef ILI9881C_EXAMPLE_H
#define ILI9881C_EXAMPLE_H
#include "stm32h7xx_hal.h"
#include "stm_lcd_ili9881c.h"
typedef struct { DSI_HandleTypeDef *dsi; GPIO_TypeDef *reset_port; uint16_t reset_pin; } lcd_ili9881c_example_board_t;
stm_err_t lcd_ili9881c_example_start(lcd_ili9881c_handle_t *panel,lcd_ili9881c_example_board_t *board);
#endif
