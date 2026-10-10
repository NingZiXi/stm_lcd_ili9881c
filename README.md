# stm_lcd_ili9881c：ILI9881C 通用面板驱动

DCS 初始化表来自慧勤智远实验 13 的 WKS101HD031-WCT 10.1 寸 800×1280 模组。来源保留，按维护者确认的 MIT 条款公开，不保证适用于其他 ILI9881C 模组。 构造函数返回 `stm_lcd_panel_handle_t`，应用通过 `stm_lcd_panel_*` 使用；不保留旧芯片句柄包装。板级拥有总线、供电、背光与 GPIO；DSI/LTDC 时序和扫描帧缓冲不写入芯片驱动。

## 🤖 让 Agent 帮助接入

> 将 `stm_lcd_ili9881c` 接入当前工程。先读 AGENTS.md、README、公开头和 HAL 示例，核对 MCU、器件、总线、引脚及尺寸/方向，保留已有改动，不猜接线。使用通用句柄和配套本地框架，按组件协议提供 IO，不增加芯片专用 LVGL 包装。报告实际源码版本、软件验证和未验证项，未经确认不烧录或发布。

## 最小接入

先由板级初始化持续有效的通用 IO（HAL 示例已提供适配函数），再创建设备：

```c
stm_lcd_panel_handle_t panel = NULL;
lcd_ili9881c_config_t config =
{
    .io = &board_io,
    .width = board_width,
    .height = board_height,
    .delay_ms = board_delay,
    .reset = board_reset,
    .control_context = &board,
};
stm_err_t err = lcd_ili9881c_create(&config, &panel);
if (err == STM_OK)
{
    err = stm_lcd_panel_reset(panel);
}
if (err == STM_OK)
{
    err = stm_lcd_panel_init(panel);
}
// 成功后创建 port；没有复位回调时跳过 reset，由板级保证已复位。
```

复位和初始化开始即清除就绪状态，中途失败不允许绘图或显示控制，可重试初始化。参数检查失败不访问硬件；绘图失败停止后续传输，不回滚已经发送的像素，也不自动重试。

## DSI 命令与可选帧缓冲能力

IO 必须提供同步 `tx_param`。`reset` 保留 1/5 ms → 0/10 ms → 1/120 ms，`init` 不隐式复位；原 DCS 命令表、Sleep Out/Display On 和 120 ms 延时不变。

ILI9881C 的 DSI 命令口本身不等于像素传输。每个实例按其 IO 实际能力提供：

| 平台 IO | 面板/port 用法 |
| --- | --- |
| 仅 `tx_param` | 允许 DCS 初始化；绘图/切帧返回 NOT_SUPPORTED，port 创建拒绝 |
| 加 `draw_region` | 默认 PARTIAL；同步复制紧密 RGB565 局部矩形至平台扫描帧缓冲 |
| 加 `present/process/busy/stop_scanout` | DIRECT；两个完整 RGB565 帧，经 VSYNC 安全切换 |

驱动不实现假成功的绘图、不创建 framebuffer、不处理 LTDC/DSI 时钟或 DCache。`draw_region` 返回前用完输入；`present` 完成代表旧扫描帧可重用，新帧仍借用至下次切换或停止扫描。停止失败保留对象和缓冲。只有实现全部能力才声明支持 DIRECT，具体责任见新版 port 的 `docs/render-modes.md`。

## 从 v0.2.0 迁移

| v0.2.0 | 当前工作区 |
| --- | --- |
| `lcd_ili9881c_handle_t` | `stm_lcd_panel_handle_t` |
| 配置中的芯片传输回调与 `void *io` | `.io` 借用通用 IO，控制回调使用 `.control_context` |
| `lcd_ili9881c_reset/init/display_on_off` | `stm_lcd_panel_reset/init/display_on_off` |
| 芯片绘图/窗口接口（支持时） | `stm_lcd_panel_draw_bitmap/set_window`，右下边界不包含 |
| `lcd_ili9881c_delete(&handle)` | `stm_lcd_panel_delete(&handle)` |

```cmake
add_subdirectory(Lib/stm_lcd) # 本次迁移使用匹配的本地框架
add_subdirectory(Lib/stm_lcd_ili9881c)
# 提供 LVGL 9 target 或 lv_conf.h 和离线来源后：
add_subdirectory(Lib/stm_lvgl_port)
target_link_libraries(your_firmware PRIVATE stm_lcd_ili9881c stm_lvgl_port)
```

## 生命周期与错误

`create` 要求输出句柄初始为 `NULL`，仅分配小型控制对象、复制配置并借用 IO，不访问硬件。创建失败不发布实例；非空输出返回 `STM_ERR_INVALID_STATE` 并保留原值。回调和上下文必须持续有效，IO、HAL、GPIO 与像素缓冲归应用所有。

删除顺序为 port → 设备 → IO → HAL/外部缓冲。删除空句柄成功，空句柄地址返回 `STM_ERR_INVALID_ARG`；被借用、正在传输或回调期间拒绝删除，实例保留供后续服务/重试。应用串行调用，禁止 ISR/递归访问；删除后自行清除其他别名。

所有操作/复位/传输回调返回 `stm_err_t`，以 `err != STM_OK` 判断错误，底层错误原样传递。延时回调保持 `void`。HAL 示例映射 `HAL_TIMEOUT` 为 `STM_ERR_TIMEOUT`，`HAL_ERROR/HAL_BUSY` 为 `STM_ERR_IO`。

| 情况 | 错误 |
| --- | --- |
| 空指针、空或倒置矩形、非法调用参数 | `STM_ERR_INVALID_ARG` |
| 缺少必需 IO 能力、尺寸/布尔配置非法 | `STM_ERR_INVALID_CONFIG` |
| 重复创建、未初始化、对象被借用或操作在途 | `STM_ERR_INVALID_STATE` |
| 控制对象分配失败 | `STM_ERR_NO_MEM` |
| 绘图越界或字节数溢出 | `STM_ERR_OUT_OF_RANGE` |
| 未实现的可选能力 | `STM_ERR_NOT_SUPPORTED` |
| 协议点数/触点 ID 校验失败 | `STM_ERR_VERIFY` |

## CMake 与离线依赖

公开链接 `stm_common` 与 `stm_lcd`，核心不依赖 HAL、LVGL 或日志。`stm_common` 解析顺序为已有 target → 同级源码 → 固定 v1.0.0 提交 `ce3d186dde2d374a8e9c7b9068a7b88f97d57dc1`；自动获取支持 `FETCHCONTENT_SOURCE_DIR_STM_COMMON` 离线覆盖、`STM_COMMON_FETCH=OFF` 和 `STM_COMMON_GIT_REPOSITORY` 镜像。

`stm_lcd` 解析顺序为已有 target → `STM_LCD_SOURCE_DIR` → `FETCHCONTENT_SOURCE_DIR_STM_LCD` → 同级源码 → 固定 v1.0.0 提交 `c359e54a657be38aec90c797ea19ee3d492d9284`。`STM_LCD_FETCH=OFF` 禁止下载；`STM_LCD_GIT_REPOSITORY` 可指向 GitHub/Gitee 镜像，默认固定 SHA 不改变。无效显式目录直接报错，不退回网络；多个组件使用同一 `stm_lcd` target/FetchContent 名称。

当前迁移组合的 ILI9881C、FT5206、GT9271 与新版 port 使用尚未发布的帧缓冲/原子寄存器扩展，**必须一起提供匹配的 `stm_lcd` 源码（聚合仓库 gitlink 固定）**；已发布 v1.0.0 不具备这些能力，配置时明确报错。ST7789/ST7796 核心仍可使用该正式框架的同步接口。依赖不自动追踪 main，也不伪造未来版本 SHA。

## 软件验证与版本边界

```sh
cmake -S tests -B build/tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

测试保留原协议用例，补充通用句柄、空参数/非法配置、重复创建、分配失败、借用回滚、删除重建、多实例与错误传递；公共头按 C11/C++17 消费。中文 HAL 示例见 [examples/stm32_hal](examples/stm32_hal/README.md)。同级新版 port 的集成测试将五种器件交给同一份 port 源码，并检查 PARTIAL/DIRECT 及失败路径。

当前提交是尚未发布新版本的通用接口迁移，原 `v0.2.0` tag 保留原 API，未发布新 tag 或 Release。

2026-10-11，匹配的本地 stm_lcd、ILI9881C、GT9271、stm_lvgl_port 和 LVGL 9.3.0 在 H757 配套 10.1 寸模组上通过 DIRECT 诊断 Debug 回归：800×1280 RGB565、DSI 两通道、板级 DMA2D，ST-Link 双核烧录独立读回、刷新持续推进、触摸/按钮事件和五次软件复位均正常，用户确认画面与触摸正常。触摸板级使用 PB10/PB11 软件 I2C，7 位地址 0x5d，mirror_x/y=0。

本次未重新验证 PARTIAL、Release、Widgets、掉电复位或长期稳定性，未独立量化各角坐标；HAL 硬件 I2C 示例仍为编译验证。其他模组/平台不在本次实板范围内，旧 tag 的验收结果不移用。固件、源码哈希和硬件日志留在消费工程本地构建目录。

## 许可证

[MIT](LICENSE)，保留维护者和既有来源说明。
