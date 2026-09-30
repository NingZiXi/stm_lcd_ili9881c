/** @file example.c @brief DSI DCS 同步传输与 reset/init 顺序。 */
#include "example.h"
#include <limits.h>
static stm_err_t hal_result(HAL_StatusTypeDef status) {
    if (status == HAL_OK) return STM_OK;
    return status == HAL_TIMEOUT ? STM_ERR_TIMEOUT : STM_ERR_IO;
}
static void delay_ms(void *io,uint32_t ms) { (void)io; HAL_Delay(ms); }
static stm_err_t reset(void *io,int high) {
    lcd_ili9881c_example_board_t *board=io;
    HAL_GPIO_WritePin(board->reset_port,board->reset_pin,high ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return STM_OK;
}
static stm_err_t transmit(void *io,uint8_t command,const uint8_t *data,size_t length) {
    lcd_ili9881c_example_board_t *board=io;
    if ((length && !data) || length>UINT32_MAX) return STM_ERR_INVALID_ARG;
    HAL_StatusTypeDef status;
    if (!length) status=HAL_DSI_ShortWrite(board->dsi,0,DSI_DCS_SHORT_PKT_WRITE_P0,command,0);
    else if (length==1) status=HAL_DSI_ShortWrite(board->dsi,0,DSI_DCS_SHORT_PKT_WRITE_P1,command,data[0]);
    else status=HAL_DSI_LongWrite(board->dsi,0,DSI_DCS_LONG_PKT_WRITE,(uint32_t)length,command,(uint8_t *)data);
    return hal_result(status);
}
stm_err_t lcd_ili9881c_example_start(lcd_ili9881c_handle_t *panel,lcd_ili9881c_example_board_t *board) {
    if (!panel || !board) return STM_ERR_INVALID_ARG;
    if (*panel) return STM_ERR_INVALID_STATE;
    if (!board->dsi || !board->reset_port) return STM_ERR_INVALID_CONFIG;
    const lcd_ili9881c_config_t cfg={.tx_param=transmit,.delay_ms=delay_ms,.reset=reset,.io=board};
    stm_err_t err=lcd_ili9881c_create(&cfg,panel);
    if (err==STM_OK) err=lcd_ili9881c_reset(*panel);
    if (err==STM_OK) err=lcd_ili9881c_init(*panel);
    if (err!=STM_OK) lcd_ili9881c_delete(panel);
    return err;
}
