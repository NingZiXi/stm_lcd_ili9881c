#include "stm_lcd_ili9881c.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
static unsigned total, delays, resets, short_cmds;
static uint8_t first, last;
static int fail;
static int tx(void *ctx, uint8_t cmd, const uint8_t *data, size_t size) {
    (void)ctx;
    if (!total) first = cmd;
    ++total;
    last = cmd;
    if (!size) { assert(!data); ++short_cmds; }
    return fail ? -1 : 0;
}
static void delay(void *ctx, uint32_t ms) { (void)ctx; assert(ms); ++delays; }
static void reset(void *ctx, int high) { (void)ctx; assert(high == 0 || high == 1); ++resets; }
int main(void) {
    stm_lcd_ili9881c_t panel = {0};
    const stm_lcd_ili9881c_config_t cfg = { tx, delay, reset, NULL };
    assert(stm_lcd_ili9881c_init(&panel) == -1);
    assert(stm_lcd_ili9881c_new_panel(NULL, &cfg) == -1);
    assert(stm_lcd_ili9881c_new_panel(&panel, &cfg) == 0);
    fail = 1;
    assert(stm_lcd_ili9881c_init(&panel) == -2 && !panel.initialized);
    fail = 0; total = 0;
    assert(stm_lcd_ili9881c_init(&panel) == 0);
    assert(first == 0xff && last == 0x29 && total > 100 && short_cmds >= 2);
    assert(resets == 6 && delays >= 5);
    assert(stm_lcd_ili9881c_display_on_off(&panel, 0) == 0 && last == 0x28);
    assert(stm_lcd_ili9881c_display_on_off(&panel, 1) == 0 && last == 0x29);
    fail = 1;
    assert(stm_lcd_ili9881c_display_on_off(&panel, 1) == -2);
    return 0;
}
