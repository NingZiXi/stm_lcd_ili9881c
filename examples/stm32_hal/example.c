/* Copy into a HAL application which has initialized DSI and its GPIO clocks. */
#include "stm32h7xx_hal.h"
#include "stm_lcd_ili9881c.h"
typedef struct {
    DSI_HandleTypeDef *dsi;
    GPIO_TypeDef *reset_port;
    uint16_t reset_pin;
} panel_example_io_t;
static void panel_delay(void *context, uint32_t ms) { (void)context; HAL_Delay(ms); }
static void panel_reset(void *context, int high) {
    panel_example_io_t *io = context;
    HAL_GPIO_WritePin(io->reset_port, io->reset_pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static int panel_transmit(void *context, uint8_t command, const uint8_t *bytes, size_t length) {
    panel_example_io_t *io = context;
    HAL_StatusTypeDef result;
    if (length == 0) result = HAL_DSI_ShortWrite(io->dsi, 0, DSI_DCS_SHORT_PKT_WRITE_P0, command, 0);
    else if (length == 1 && bytes) result = HAL_DSI_ShortWrite(io->dsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, command, bytes[0]);
    else if (length > 1 && bytes && length <= UINT32_MAX)
        result = HAL_DSI_LongWrite(io->dsi, 0, DSI_DCS_LONG_PKT_WRITE, (uint32_t)length, command, (uint8_t *)bytes);
    else return -1;
    return result == HAL_OK ? 0 : -1;
}
int example_ili9881c_start(stm_lcd_ili9881c_t *panel, panel_example_io_t *io) {
    stm_lcd_ili9881c_config_t cfg = {
        .tx_param = panel_transmit, .reset = panel_reset, .delay_ms = panel_delay, .io = io,
    };
    if (!panel || !io || !io->dsi || !io->reset_port) return -1;
    if (stm_lcd_ili9881c_new_panel(panel, &cfg)) return -1;
    return stm_lcd_ili9881c_init(panel);
}
