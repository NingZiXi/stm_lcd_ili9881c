/**
 * @file example.h
 * @brief DSI 模组与可选 LTDC 能力，时钟与供电由板级管理。
 */
#ifndef STM_LCD_ILI9881C_EXAMPLE_H
#define STM_LCD_ILI9881C_EXAMPLE_H
#include "stm32h7xx_hal.h"
#include "stm_lcd_ili9881c.h"
#include "stm_lcd_impl.h"

typedef struct
{
    struct stm_lcd_io io; // 板级持有，首次使用必须清零。
    stm_lcd_io_ops_t ops; // 本实例操作表副本，命令函数由示例设置。
    const stm_lcd_io_ops_t *platform_ops; // 提供 draw_region 或 present/process/busy/stop_scanout。
    void *platform_context;               // 板级回调从本结构体获取其 LTDC 上下文。
    DSI_HandleTypeDef *dsi;               // 借用的已初始化 DSI。
    GPIO_TypeDef *reset_port;             // 可选模组复位口。
    uint16_t reset_pin;                   // 模组复位引脚。
    uint16_t width, height;               // 模组逻辑尺寸，未指定板级默认值。
} lcd_ili9881c_example_board_t;

/**
 * @brief 建立板级 IO 并创建、复位设备，面板完成初始化。
 * @param device 初始为空的通用句柄地址
 * @param board 已清零 IO 且 HAL 已初始化的板级配置，须持续有效
 * @return STM_OK；失败回收本次创建资源，底层错误原样传递。
 */
stm_err_t lcd_ili9881c_example_start(stm_lcd_panel_handle_t *device,
                                     lcd_ili9881c_example_board_t *board);
/**
 * @brief 删除设备后清除板级 IO，不释放 HAL 或帧缓冲。
 * @param device 已先删除 port 的句柄地址，成功清空
 * @param board 创建时使用的板级配置
 * @return STM_OK；在途、被借用或停止失败时保留资源供重试。
 */
stm_err_t lcd_ili9881c_example_stop(stm_lcd_panel_handle_t *device,
                                    lcd_ili9881c_example_board_t *board);
#endif
