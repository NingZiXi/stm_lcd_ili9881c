# STM32 HAL 接入示例

在 CubeMX 中启用 DSI Host 双通道及对应的 LTDC RGB565，并为屏幕供电、复位和背光配置 GPIO；先初始化 SDRAM、DSI 与 LTDC，再使用以下代码。用户须按实际电路实现三个板级函数，不要将 GPIO 引脚写进芯片组件。`tx_param` 对无参数 DCS 用 short write；带参数的命令按参数长度选 short/long write。

```c
static int panel_tx(void *io, uint8_t cmd, const uint8_t *data, size_t length);
static void panel_delay(void *io, uint32_t ms);
static void panel_reset(void *io, int high);

stm_lcd_ili9881c_t panel = {0};
stm_lcd_ili9881c_config_t cfg = {
    .tx_param = panel_tx, .delay_ms = panel_delay, .reset = panel_reset,
};
if (stm_lcd_ili9881c_new_panel(&panel, &cfg) == 0 &&
    stm_lcd_ili9881c_init(&panel) == 0) {
    /* 然后打开板级背光；像素由 LTDC 帧缓冲提供。 */
}
```

直接可构建的 DSI/LTDC、GPIO 时序、LVGL 刷新参考 STM32H757 主工程 `examples/display_lvgl_demo.c`（使用 `STM_DISPLAY_LVGL_DEMO=ON`）。

`example.c` 提供 HAL DSI 传输回调与初始化函数；请先按板级时序启动 DSI，调用 `example_ili9881c_start()` 后再开启背光。
