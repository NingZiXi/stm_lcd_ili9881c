# ILI9881C DSI 屏幕组件

提供 `stm_lcd_ili9881c_new_panel()`、`stm_lcd_ili9881c_init()`、`stm_lcd_ili9881c_display_on_off()`。命令表来自慧勤智远 10.1 寸 WKS101HD031-WCT 模组示例，**不保证适合其他 ILI9881C 模组**。本组件仅管理 DCS 初始化和开关屏；板级代码提供 DSI 发送、复位、延时和 LTDC/SDRAM 帧缓冲，组件不依赖 HAL/LVGL。

```cmake
add_subdirectory(lib/stm_lcd_ili9881c)
target_link_libraries(your_app PRIVATE stm_lcd_ili9881c)
```

在板级实现同步回调 `tx_param(io, command, data, length)`：0 表示成功；`length=0` 是无参数 DCS 命令。初始化成功返回 0；`-1` 为未创建/参数错误，`-2` 为命令传输失败。先启动 DSI/LTDC，再初始化面板，最后打开背光。可复制的板级接入见 [examples/stm32_hal](examples/stm32_hal/)；完整 STM32H757 可选 LVGL 示例在主工程 `examples/display_lvgl_demo.c`。

运行 `cmake -S tests -B build/tests && cmake --build build/tests && ctest --test-dir build/tests --output-on-failure` 检查命令及错误路径。已有该开发板基础色块实测；LVGL 联动验收应以主工程实测记录为准。组件以 MIT 许可证发布；模组初始化表转录自板厂实验 13，来源在源码注明，项目维护者已确认按 MIT 方式公开。命令表只针对已测试模组，不保证适用于其他 ILI9881C 屏。
