/**
 * @file stm_lcd_ili9881c.h
 * @brief ILI9881C 通用面板构造接口。
 */
#ifndef STM_LCD_ILI9881C_H
#define STM_LCD_ILI9881C_H
#include "stm_lcd.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct
{
    stm_lcd_io_handle_t io;             // 借用的初始化 IO。
    uint16_t width, height;             // 逻辑 RGB565 尺寸。
    void (*delay_ms)(void *, uint32_t); // 必需阻塞延时。
    stm_err_t (*reset)(void *, int);    // 可选 RST 电平。
    void *control_context;              // 借用控制回调上下文。
} lcd_ili9881c_config_t;

/**
 * @brief 创建通用面板，不访问硬件；按 reset → init 显式启动。
 * @param config 复制配置，IO 与回调上下文由应用持有
 * @param out 初始为空的输出句柄地址，失败保持为空
 * @return STM_OK 或参数、配置、分配及 IO 借用错误。
 */
stm_err_t lcd_ili9881c_create(const lcd_ili9881c_config_t *config, stm_lcd_panel_handle_t *out);
#ifdef __cplusplus
}
#endif
#endif
