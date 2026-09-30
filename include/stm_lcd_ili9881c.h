#ifndef STM_LCD_ILI9881C_H
#define STM_LCD_ILI9881C_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Controller command layer for WKS101HD031-WCT (800 x 1280, two DSI lanes).
 * The application owns DSI Host, LTDC, SDRAM framebuffer, power and backlight. */
typedef struct {
    /* Send DSI DCS command plus data, synchronously; 0 on success. */
    int (*tx_param)(void *io, uint8_t command, const uint8_t *data, size_t length);
    void (*delay_ms)(void *io, uint32_t milliseconds);
    void (*reset)(void *io, int high);
    void *io;
} stm_lcd_ili9881c_config_t;
typedef struct { stm_lcd_ili9881c_config_t config; uint8_t created, initialized; } stm_lcd_ili9881c_t;
int stm_lcd_ili9881c_new_panel(stm_lcd_ili9881c_t *panel, const stm_lcd_ili9881c_config_t *config);
int stm_lcd_ili9881c_init(stm_lcd_ili9881c_t *panel);
int stm_lcd_ili9881c_display_on_off(stm_lcd_ili9881c_t *panel, int on);
#ifdef __cplusplus
}
#endif
#endif
