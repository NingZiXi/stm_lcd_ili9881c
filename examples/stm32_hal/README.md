# ILI9881C STM32 HAL 接入示例

源码为 [example.c](example.c) 和 [example.h](example.h)，使用真实 STM32H7 HAL 头文件；消费工程需先完成 DSI、LTDC、供电/背光、逻辑尺寸、可选 RST，以及平台像素/扫描能力。 示例不猜引脚、时钟或屏幕通道。

## 添加与启动

将 `example.c` 编入消费工程，并添加本目录 include，链接 `stm_lcd_ili9881c` 与实际 HAL target。组件间依赖按[组件 README](../../README.md)设置，使用本次迁移匹配的本地框架。

```c
static lcd_ili9881c_example_board_t board; // 初始化硬件字段；内嵌 IO 首次必须为零。
static stm_lcd_panel_handle_t device = NULL;
// 填写 board 的 HAL、引脚、尺寸等实际配置后：
stm_err_t err = lcd_ili9881c_example_start(&device, &board);
// 成功时把通用句柄传给 port；板级 board 不得离开作用域。
```

`example_start` 建立通用 IO、创建设备、可选硬复位；面板还执行 init。失败回收本次拥有的设备和 IO，HAL/引脚/帧缓冲归应用。HAL_TIMEOUT → TIMEOUT，HAL_ERROR/HAL_BUSY → IO，驱动不吞错误。

## LTDC/DSI 平台操作

`board.platform_ops` 提供 `draw_region`，或提供完整的 `present/process/busy/stop_scanout`。示例复制该表到 `board.ops` 并替换命令函数为真实 HAL DSI DCS 调用；所有平台回调的 context 为 `lcd_ili9881c_example_board_t *`，通过其 `platform_context` 取板级 LTDC 状态。

PARTIAL 回调同步复制 `[x1,x2) × [y1,y2)` 紧密 RGB565 输入至板级 framebuffer，处理行跨度/缓存，返回前用完输入。DIRECT 提交完整帧；VSYNC IRQ 只记录邮箱，主循环确认旧帧停止扫描后调用 `stm_lcd_io_complete(&board.io, result)` 恰好一次。成功后新帧继续被扫描；`busy` 只表示在途换帧，不能把持续扫描当成一直忙。`stop_scanout` 成功后 HAL 不得再引用任意缓冲。

不能用空成功函数代替像素能力；只有 DCS 能力时可初始化屏幕，但 port 创建会拒绝。`example_stop` 会先停止扫描，失败时保留设备/IO。具体缓存、VSYNC、双缓冲契约见新版 port 的渲染说明。

## 接入同一个 LVGL port 与退出

port 配置直接填写 `.io`、`.panel`、可选 `.touch`、外部缓冲和时钟，不增加芯片专用 LVGL 绘图/输入包装。删除顺序：先 `lvgl_port_delete(&port)`，然后 `lcd_ili9881c_example_stop(&device, &board)`；该函数删除设备后 deinit 内嵌 IO，不释放 HAL。在途/被借用/停止失败时保留资源，继续服务并重试，不清零有效对象。

本示例经过主机/真实 HAL 编译检查。2026-10-11 H757 配套屏幕消费工程使用相同通用组件通过显示/触摸与五次软件复位，具体范围见[组件 README](../../README.md)。板级工程使用自己的 DSI/LTDC 和软件 I2C 适配，不能将该结论当作本示例所有 HAL 操作或接线的实板验收。
