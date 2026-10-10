/**
 * @file example.c
 * @brief DSI 命令与 LTDC 板级能力接入。
 */
#include "example.h"
#include <limits.h>

static stm_err_t hal_result(HAL_StatusTypeDef status)
{
    if (status == HAL_OK)
    {
        return STM_OK;
    }
    return status == HAL_TIMEOUT ? STM_ERR_TIMEOUT : STM_ERR_IO;
}

static void delay_ms(void *io, uint32_t ms)
{
    (void)io;
    HAL_Delay(ms);
}

static stm_err_t reset(void *io, int high)
{
    lcd_ili9881c_example_board_t *board = io;
    HAL_GPIO_WritePin(board->reset_port, board->reset_pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return STM_OK;
}

static stm_err_t transmit(void *io, uint8_t command, const uint8_t *data, size_t length)
{
    lcd_ili9881c_example_board_t *board = io;
    if ((length && !data) || length > UINT32_MAX)
    {
        return STM_ERR_INVALID_ARG;
    }
    HAL_StatusTypeDef status;
    if (!length)
    {
        status = HAL_DSI_ShortWrite(board->dsi, 0, DSI_DCS_SHORT_PKT_WRITE_P0, command, 0);
    }
    else if (length == 1)
    {
        status = HAL_DSI_ShortWrite(board->dsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, command, data[0]);
    }
    else
    {
        status = HAL_DSI_LongWrite(board->dsi, 0, DSI_DCS_LONG_PKT_WRITE, (uint32_t)length, command,
                                   (uint8_t *)data);
    }
    return hal_result(status);
}

stm_err_t lcd_ili9881c_example_start(stm_lcd_panel_handle_t *panel,
                                     lcd_ili9881c_example_board_t *board)
{
    if (!panel || !board)
    {
        return STM_ERR_INVALID_ARG;
    }
    if (*panel || board->io.ops)
    {
        return STM_ERR_INVALID_STATE;
    }
    if (!board->dsi || !board->width || !board->height || !board->platform_ops)
    {
        return STM_ERR_INVALID_CONFIG;
    }
    board->ops = *board->platform_ops;
    board->ops.tx_param = transmit;
    const lcd_ili9881c_config_t cfg = {.io = &board->io,
                                       .control_context = board,
                                       .width = board->width,
                                       .height = board->height,
                                       .delay_ms = delay_ms,
                                       .reset = board->reset_port ? reset : NULL};
    stm_err_t err = stm_lcd_io_init(&board->io, &board->ops, board);
    if (err == STM_OK)
    {
        err = lcd_ili9881c_create(&cfg, panel);
    }
    if (err == STM_OK && board->reset_port)
    {
        err = stm_lcd_panel_reset(*panel);
    }
    if (err == STM_OK)
    {
        err = stm_lcd_panel_init(*panel);
    }
    if (err != STM_OK)
    {
        (void)stm_lcd_panel_delete(panel);
        if (board->io.ops)
        {
            (void)stm_lcd_io_deinit(&board->io);
        }
    }
    return err;
}

stm_err_t lcd_ili9881c_example_stop(stm_lcd_panel_handle_t *panel,
                                    lcd_ili9881c_example_board_t *board)
{
    if (!panel || !board)
    {
        return STM_ERR_INVALID_ARG;
    }
    stm_err_t err = *panel ? stm_lcd_panel_stop_scanout(*panel) : STM_OK;
    if (err == STM_OK)
    {
        err = stm_lcd_panel_delete(panel);
    }
    if (err == STM_OK && board->io.ops)
    {
        err = stm_lcd_io_deinit(&board->io);
    }
    return err;
}
