# stm_lcd_ili9881c：ILI9881C DSI 面板驱动

管理 DCS 初始化和开关显示，命令表来自慧勤智远实验 13 的 WKS101HD031-WCT 10.1 寸 800×1280 模组。来源保留，按维护者确认的 MIT 条款公开；不保证适用于其他 ILI9881C 模组。DSI/LTDC、上电、背光和帧缓冲由板级管理。

## 最小调用

```c
lcd_ili9881c_handle_t panel = NULL;
lcd_ili9881c_config_t cfg = {
    .tx_param=board_dsi_send, .reset=board_reset, .delay_ms=board_delay_ms, .io=&board_io,
};
stm_err_t err = lcd_ili9881c_create(&cfg, &panel);
if (err == STM_OK) err = lcd_ili9881c_reset(panel);
if (err == STM_OK) err = lcd_ili9881c_init(panel);
if (err != STM_OK) lcd_ili9881c_delete(&panel);
/* 初始化成功后由板级打开背光；退出时 lcd_ili9881c_delete(&panel)。 */
```

## 错误与资源契约

所有操作和传输/复位回调返回 `stm_err_t`，成功为 `STM_OK`，失败检查 `err != STM_OK`，不能使用 `err < 0`。HAL 适配将 `HAL_TIMEOUT` 映射为 `STM_ERR_TIMEOUT`，`HAL_ERROR/HAL_BUSY` 映射为 `STM_ERR_IO`；组件原样传递回调错误，延时回调仍返回 void。

| 情况 | 错误 |
| --- | --- |
| 空参数、非法调用参数 | `STM_ERR_INVALID_ARG` |
| 缺少必需回调、尺寸或方向配置错误 | `STM_ERR_INVALID_CONFIG` |
| 输出句柄非空、面板未初始化 | `STM_ERR_INVALID_STATE` |
| 控制对象/LVGL 对象分配失败 | `STM_ERR_NO_MEM` |
| 绘图越界或像素长度计算溢出 | `STM_ERR_OUT_OF_RANGE` |
| GT9271 ID 不匹配 | `STM_ERR_NOT_SUPPORTED` |
| 触摸帧点数等数据校验失败 | `STM_ERR_VERIFY` |

`create(config, &handle)` 要求 handle 初始为 NULL；复制配置，用 calloc/free 管理小型控制对象，芯片 create 不访问硬件。创建失败保持输出为空；非空输出被拒绝且原值不变。`delete(&handle)` 仅回收拥有的对象，成功清空 handle，空句柄也成功；NULL 句柄地址是参数错误。删除前停止并发访问，其他别名不会被自动清空。

板级拥有 HAL、总线、GPIO、背光、外部缓冲和回调上下文；组件不释放或重新配置这些资源。实例使用期间上下文必须有效，可用 NULL io 表示无上下文。应用串行调用，组件不默认线程安全，不在中断中调用阻塞操作，不增加日志/RTT/RTOS 依赖。同步传输返回前必须用完输入缓冲；共享总线在整笔事务外加锁，DMA/DCache 一致性由板级管理。

## 初始化与失败状态

init 不再隐式 reset；应用按 create → reset → init 调用一次复位，RST 保持原有 1/5ms → 0/10ms → 1/120ms，命令序列与延时不变。无 reset 回调时由板级保证实际复位。

复位/初始化开始后未就绪，失败保持未就绪，可重新初始化；仅初始化成功后允许开关显示。tx_param 是同步 DCS 回调，长度以字节计，length=0 时 data 可空；回调返回前使用完输入，错误原样返回，不自动重试。

## CMake 与依赖

依赖 `stm_common` 的 `stm_err.h`，不复制公共错误码。优先复用已有 `stm_common` target，其次找同级源码；缺失时自动下载固定 v1.0.0 提交 `ce3d186dde2d374a8e9c7b9068a7b88f97d57dc1`。可设置 `STM_COMMON_FETCH=OFF` 禁止下载，`STM_COMMON_GIT_REPOSITORY=https://gitee.com/nzxhg/stm_common.git` 指定镜像，或 `FETCHCONTENT_SOURCE_DIR_STM_COMMON` 指定离线源码。已有 target/同级源码无需网络。

```cmake
add_subdirectory(Lib/stm_lcd_ili9881c)
target_link_libraries(your_firmware PRIVATE stm_lcd_ili9881c)
```

手动集成时添加组件 include/源码及 stm_common 头文件目录。LVGL port 还要求应用提前提供 LVGL 9 的 `lvgl` target 和配置。

## 从 v0.1.0 迁移

| 旧接口 | 当前接口 |
| --- | --- |
| `stm_lcd_ili9881c_t` 公开结构体 | `lcd_ili9881c_handle_t`，初始 NULL |
| `stm_lcd_ili9881c_config_t` | `lcd_ili9881c_config_t` |
| `new_panel(实例地址, config)` | `lcd_ili9881c_create(config, &handle)` |
| 直接访问结构体 / 无销毁接口 | `lcd_ili9881c_delete(&handle)` |
| int 与负数错误码 | `stm_err_t`，`err != STM_OK`，回调同步迁移 |

其他操作使用 lcd_<型号> 前缀。

v0.1.0 已在 H757 配套模组验证颜色、LVGL 刷新与连续复位；本次 API 迁移尚未实板回归。

## 软件验证与发布状态

```sh
cmake -S tests -B build/tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

主机测试覆盖参数/配置、分配失败、资源回收、多实例和错误传递，并编译 C11/C++17 公共头文件。测试分配器仅用于测试构建，不加入产品固件。中文 HAL 示例见 [examples/stm32_hal/README.md](examples/stm32_hal/README.md)。许可证见 [LICENSE](LICENSE)。

当前为未发布的 API 软件迁移；已发布 `v0.1.0` 保留旧接口，迁移后的硬件回归待完成，尚未发布 v0.2.0。
