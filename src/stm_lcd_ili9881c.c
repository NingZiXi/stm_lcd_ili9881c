/**
 * @file stm_lcd_ili9881c.c
 * @brief 面板命令与实例生命周期。
 */
#include "stm_lcd_ili9881c.h"
#include "stm_lcd_impl.h"
#ifndef STM_LCD_FRAMEBUFFER_API
#error                                                                                             \
    "This unpublished migration requires the matching local stm_lcd source with framebuffer/register capabilities"
#endif
#include <stdlib.h>

/* Panel-specific register values transcribed from board vendor experiment 13,
 * ili9881c10_init_cmd_tbl; do not apply to other ILI9881C modules blindly. */
typedef struct
{
    uint8_t command, length, data[3]; // 原始 DCS 命令、长度与参数。
} init_cmd_t;

static const init_cmd_t init_commands[] = {
    {0xFF, 3, {0x98, 0x81, 0x03}},
    {0x01, 1, {0x00}},
    {0x02, 1, {0x00}},
    {0x03, 1, {0x53}},
    {0x04, 1, {0x53}},
    {0x05, 1, {0x13}},
    {0x06, 1, {0x04}},
    {0x07, 1, {0x02}},
    {0x08, 1, {0x02}},
    {0x09, 1, {0x00}},
    {0x0A, 1, {0x00}},
    {0x0B, 1, {0x00}},
    {0x0C, 1, {0x00}},
    {0x0D, 1, {0x00}},
    {0x0E, 1, {0x00}},
    {0x0F, 1, {0x00}},
    {0x10, 1, {0x00}},
    {0x11, 1, {0x00}},
    {0x12, 1, {0x00}},
    {0x13, 1, {0x00}},
    {0x14, 1, {0x00}},
    {0x15, 1, {0x00}},
    {0x16, 1, {0x00}},
    {0x17, 1, {0x00}},
    {0x18, 1, {0x00}},
    {0x19, 1, {0x00}},
    {0x1A, 1, {0x00}},
    {0x1B, 1, {0x00}},
    {0x1C, 1, {0x00}},
    {0x1D, 1, {0x00}},
    {0x1E, 1, {0xC0}},
    {0x1F, 1, {0x00}},
    {0x20, 1, {0x02}},
    {0x21, 1, {0x09}},
    {0x22, 1, {0x00}},
    {0x23, 1, {0x00}},
    {0x24, 1, {0x00}},
    {0x25, 1, {0x00}},
    {0x26, 1, {0x00}},
    {0x27, 1, {0x00}},
    {0x28, 1, {0x55}},
    {0x29, 1, {0x03}},
    {0x2A, 1, {0x00}},
    {0x2B, 1, {0x00}},
    {0x2C, 1, {0x00}},
    {0x2D, 1, {0x00}},
    {0x2E, 1, {0x00}},
    {0x2F, 1, {0x00}},
    {0x30, 1, {0x00}},
    {0x31, 1, {0x00}},
    {0x32, 1, {0x00}},
    {0x33, 1, {0x00}},
    {0x34, 1, {0x00}},
    {0x35, 1, {0x00}},
    {0x36, 1, {0x00}},
    {0x37, 1, {0x00}},
    {0x38, 1, {0x3C}},
    {0x39, 1, {0x00}},
    {0x3A, 1, {0x00}},
    {0x3B, 1, {0x00}},
    {0x3C, 1, {0x00}},
    {0x3D, 1, {0x00}},
    {0x3E, 1, {0x00}},
    {0x3F, 1, {0x00}},
    {0x40, 1, {0x00}},
    {0x41, 1, {0x00}},
    {0x42, 1, {0x00}},
    {0x43, 1, {0x00}},
    {0x44, 1, {0x00}},
    {0x45, 1, {0x00}},
    {0x50, 1, {0x01}},
    {0x51, 1, {0x23}},
    {0x52, 1, {0x45}},
    {0x53, 1, {0x67}},
    {0x54, 1, {0x89}},
    {0x55, 1, {0xAB}},
    {0x56, 1, {0x01}},
    {0x57, 1, {0x23}},
    {0x58, 1, {0x45}},
    {0x59, 1, {0x67}},
    {0x5A, 1, {0x89}},
    {0x5B, 1, {0xAB}},
    {0x5C, 1, {0xCD}},
    {0x5D, 1, {0xEF}},
    {0x5E, 1, {0x01}},
    {0x5F, 1, {0x0A}},
    {0x60, 1, {0x02}},
    {0x61, 1, {0x02}},
    {0x62, 1, {0x08}},
    {0x63, 1, {0x15}},
    {0x64, 1, {0x14}},
    {0x65, 1, {0x02}},
    {0x66, 1, {0x11}},
    {0x67, 1, {0x10}},
    {0x68, 1, {0x02}},
    {0x69, 1, {0x0F}},
    {0x6A, 1, {0x0E}},
    {0x6B, 1, {0x02}},
    {0x6C, 1, {0x0D}},
    {0x6D, 1, {0x0C}},
    {0x6E, 1, {0x06}},
    {0x6F, 1, {0x02}},
    {0x70, 1, {0x02}},
    {0x71, 1, {0x02}},
    {0x72, 1, {0x02}},
    {0x73, 1, {0x02}},
    {0x74, 1, {0x02}},
    {0x75, 1, {0x0A}},
    {0x76, 1, {0x02}},
    {0x77, 1, {0x02}},
    {0x78, 1, {0x06}},
    {0x79, 1, {0x15}},
    {0x7A, 1, {0x14}},
    {0x7B, 1, {0x02}},
    {0x7C, 1, {0x10}},
    {0x7D, 1, {0x11}},
    {0x7E, 1, {0x02}},
    {0x7F, 1, {0x0C}},
    {0x80, 1, {0x0D}},
    {0x81, 1, {0x02}},
    {0x82, 1, {0x0E}},
    {0x83, 1, {0x0F}},
    {0x84, 1, {0x08}},
    {0x85, 1, {0x02}},
    {0x86, 1, {0x02}},
    {0x87, 1, {0x02}},
    {0x88, 1, {0x02}},
    {0x89, 1, {0x02}},
    {0x8A, 1, {0x02}},
    {0xFF, 3, {0x98, 0x81, 0x04}},
    {0x6C, 1, {0x15}},
    {0x6E, 1, {0x30}},
    {0x6F, 1, {0x55}},
    {0x3A, 1, {0x24}},
    {0x8D, 1, {0x1F}},
    {0x87, 1, {0xBA}},
    {0x26, 1, {0x76}},
    {0xB2, 1, {0xD1}},
    {0xB5, 1, {0x07}},
    {0x35, 1, {0x1F}},
    {0x88, 1, {0x0B}},
    {0x21, 1, {0x30}},
    {0xFF, 3, {0x98, 0x81, 0x01}},
    {0x22, 1, {0x0A}},
    {0x31, 1, {0x09}},
    {0x40, 1, {0x33}},
    {0x53, 1, {0x37}},
    {0x55, 1, {0x88}},
    {0x50, 1, {0x95}},
    {0x51, 1, {0x95}},
    {0x60, 1, {0x30}},
    {0xA0, 1, {0x0F}},
    {0xA1, 1, {0x17}},
    {0xA2, 1, {0x22}},
    {0xA3, 1, {0x19}},
    {0xA4, 1, {0x15}},
    {0xA5, 1, {0x28}},
    {0xA6, 1, {0x1C}},
    {0xA7, 1, {0x1C}},
    {0xA8, 1, {0x78}},
    {0xA9, 1, {0x1C}},
    {0xAA, 1, {0x28}},
    {0xAB, 1, {0x69}},
    {0xAC, 1, {0x1A}},
    {0xAD, 1, {0x19}},
    {0xAE, 1, {0x4B}},
    {0xAF, 1, {0x22}},
    {0xB0, 1, {0x2A}},
    {0xB1, 1, {0x4B}},
    {0xB2, 1, {0x6B}},
    {0xB3, 1, {0x3F}},
    {0xB7, 1, {0x03}},
    {0xC0, 1, {0x01}},
    {0xC1, 1, {0x17}},
    {0xC2, 1, {0x22}},
    {0xC3, 1, {0x19}},
    {0xC4, 1, {0x15}},
    {0xC5, 1, {0x28}},
    {0xC6, 1, {0x1C}},
    {0xC7, 1, {0x1D}},
    {0xC8, 1, {0x78}},
    {0xC9, 1, {0x1C}},
    {0xCA, 1, {0x28}},
    {0xCB, 1, {0x69}},
    {0xCC, 1, {0x1A}},
    {0xCD, 1, {0x19}},
    {0xCE, 1, {0x4B}},
    {0xCF, 1, {0x22}},
    {0xD0, 1, {0x2A}},
    {0xD1, 1, {0x4B}},
    {0xD2, 1, {0x6B}},
    {0xD3, 1, {0x3F}},
};

struct lcd_ili9881c_context
{
    struct stm_lcd_panel base;    // 首成员，框架管理借用和完成。
    lcd_ili9881c_config_t config; // 配置副本，不拥有外部资源。
    stm_lcd_panel_ops_t ops;      // 每实例能力表，与 IO 实际能力一致。
    uint8_t initialized;          // 仅完整初始化成功后就绪。
};

static stm_err_t panel_reset(stm_lcd_panel_handle_t handle)
{
    struct lcd_ili9881c_context *panel = (struct lcd_ili9881c_context *)handle;
    if (!panel)
    {
        return STM_ERR_INVALID_ARG;
    }
    panel->initialized = 0;
    if (!panel->config.reset)
    {
        return STM_ERR_NOT_SUPPORTED;
    }
    stm_err_t err;
    err = panel->config.reset(panel->config.control_context, 1);
    if (err != STM_OK)
    {
        return err;
    }
    panel->config.delay_ms(panel->config.control_context, 5);
    err = panel->config.reset(panel->config.control_context, 0);
    if (err != STM_OK)
    {
        return err;
    }
    panel->config.delay_ms(panel->config.control_context, 10);
    err = panel->config.reset(panel->config.control_context, 1);
    if (err != STM_OK)
    {
        return err;
    }
    panel->config.delay_ms(panel->config.control_context, 120);
    return STM_OK;
}

static stm_err_t panel_init(stm_lcd_panel_handle_t handle)
{
    struct lcd_ili9881c_context *panel = (struct lcd_ili9881c_context *)handle;
    if (!panel)
    {
        return STM_ERR_INVALID_ARG;
    }
    panel->initialized = 0;
    stm_err_t err;
    for (size_t i = 0; i < sizeof(init_commands) / sizeof(init_commands[0]); ++i)
    {
        const init_cmd_t *op = &init_commands[i];
        err = stm_lcd_io_tx_param(panel->base.io, op->command, op->data, op->length);
        if (err != STM_OK)
        {
            return err;
        }
    }
    static const uint8_t page0[3] = {0x98, 0x81, 0x00}, tear_on = 0;
    err = stm_lcd_io_tx_param(panel->base.io, 0xff, page0, 3);
    if (err != STM_OK)
    {
        return err;
    }
    err = stm_lcd_io_tx_param(panel->base.io, 0x35, &tear_on, 1);
    if (err != STM_OK)
    {
        return err;
    }
    err = stm_lcd_io_tx_param(panel->base.io, 0x11, NULL, 0);
    if (err != STM_OK)
    {
        return err;
    }
    panel->config.delay_ms(panel->config.control_context, 120);
    err = stm_lcd_io_tx_param(panel->base.io, 0x29, NULL, 0);
    if (err != STM_OK)
    {
        return err;
    }
    panel->config.delay_ms(panel->config.control_context, 120);
    panel->initialized = 1;
    return STM_OK;
}

static stm_err_t panel_display_on_off(stm_lcd_panel_handle_t handle, int on)
{
    struct lcd_ili9881c_context *panel = (struct lcd_ili9881c_context *)handle;
    if (!panel || (on != 0 && on != 1))
    {
        return STM_ERR_INVALID_ARG;
    }
    if (!panel->initialized)
    {
        return STM_ERR_INVALID_STATE;
    }
    return stm_lcd_io_tx_param(panel->base.io, on ? 0x29 : 0x28, NULL, 0);
}

static stm_err_t panel_draw_bitmap(stm_lcd_panel_handle_t handle,
                                   uint16_t x1,
                                   uint16_t y1,
                                   uint16_t x2,
                                   uint16_t y2,
                                   const void *pixels)
{
    struct lcd_ili9881c_context *panel = (struct lcd_ili9881c_context *)handle;
    if (!panel->initialized)
    {
        return STM_ERR_INVALID_STATE;
    }
    return stm_lcd_io_draw_region(handle->io, x1, y1, x2, y2, pixels);
}

static stm_err_t panel_present(stm_lcd_panel_handle_t handle, const void *pixels)
{
    struct lcd_ili9881c_context *panel = (struct lcd_ili9881c_context *)handle;
    if (!panel->initialized)
    {
        return STM_ERR_INVALID_STATE;
    }
    return stm_lcd_io_present(handle->io, pixels, handle->width, handle->height);
}

static void panel_destroy(stm_lcd_panel_handle_t handle)
{
    free(handle);
}

static const stm_lcd_panel_ops_t panel_ops = {.reset = panel_reset,
                                              .init = panel_init,
                                              .display_on_off = panel_display_on_off,
                                              .draw = panel_draw_bitmap,
                                              .present = panel_present,
                                              .destroy = panel_destroy};

stm_err_t lcd_ili9881c_create(const lcd_ili9881c_config_t *config, stm_lcd_panel_handle_t *out)
{
    if (!config || !out)
    {
        return STM_ERR_INVALID_ARG;
    }
    if (*out)
    {
        return STM_ERR_INVALID_STATE;
    }
    if (!config->io || !config->io->ops || !config->io->ops->tx_param || !config->delay_ms ||
        !config->width || !config->height)
    {
        return STM_ERR_INVALID_CONFIG;
    }
    struct lcd_ili9881c_context *panel = calloc(1, sizeof(*panel));
    if (!panel)
    {
        return STM_ERR_NO_MEM;
    }
    panel->config = *config;
    panel->ops = panel_ops;
    if (!config->io->ops->draw_region)
    {
        panel->ops.draw = NULL;
    }
    if (!stm_lcd_io_supports_present(config->io))
    {
        panel->ops.present = NULL;
    }
    stm_err_t err = stm_lcd_panel_base_init(&panel->base, &panel->ops, config->io, config->width,
                                            config->height);
    if (err != STM_OK)
    {
        free(panel);
        return err;
    }
    *out = &panel->base;
    return STM_OK;
}
