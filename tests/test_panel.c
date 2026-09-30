#include "stm_lcd_ili9881c.h"
#include "test_allocator.h"
#include <assert.h>
#include <string.h>
typedef struct {
    unsigned commands, colors, fail_at, reset_calls, reset_fail_at, delays;
    uint32_t delay_values[16]; int reset_values[16];
    size_t bytes; uint8_t xwindow[4], ywindow[4], first, last;
} mock_t;
static stm_err_t cmd(void *io, uint8_t command, const uint8_t *data, size_t len) {
    mock_t *m=io;
    if (!m->commands) m->first=command;
    ++m->commands; m->last=command;
    if (command==0x2a && len==4) memcpy(m->xwindow,data,4);
    if (command==0x2b && len==4) memcpy(m->ywindow,data,4);
    return m->commands==m->fail_at ? STM_ERR_TIMEOUT : STM_OK;
}

static void delay(void *io,uint32_t ms) {
    mock_t *m=io; if (m->delays<16) m->delay_values[m->delays]=ms; ++m->delays;
}
static stm_err_t reset(void *io,int high) {
    mock_t *m=io; if (m->reset_calls<16) m->reset_values[m->reset_calls]=high;
    ++m->reset_calls; return m->reset_calls==m->reset_fail_at ? STM_ERR_IO : STM_OK;
}
int main(void) {
    mock_t m={0}, m2={0}; lcd_ili9881c_handle_t p=NULL, other=NULL;
    lcd_ili9881c_config_t cfg={.tx_param=cmd,.delay_ms=delay,.reset=reset,.io=&m}, bad;
    assert(lcd_ili9881c_create(NULL,&p)==STM_ERR_INVALID_ARG && !p);
    assert(lcd_ili9881c_create(&cfg,NULL)==STM_ERR_INVALID_ARG);
    assert(lcd_ili9881c_delete(NULL)==STM_ERR_INVALID_ARG);
    assert(lcd_ili9881c_delete(&p)==STM_OK && !p);
    assert(lcd_ili9881c_init(NULL)==STM_ERR_INVALID_ARG);
    assert(lcd_ili9881c_reset(NULL)==STM_ERR_INVALID_ARG);
    assert(lcd_ili9881c_display_on_off(NULL,0)==STM_ERR_INVALID_ARG);
    bad=cfg; bad.tx_param=NULL;
    assert(lcd_ili9881c_create(&bad,&p)==STM_ERR_INVALID_CONFIG && !p);
    bad=cfg; bad.delay_ms=NULL;
    assert(lcd_ili9881c_create(&bad,&p)==STM_ERR_INVALID_CONFIG && !p);

    test_alloc_fail=1;
    assert(lcd_ili9881c_create(&cfg,&p)==STM_ERR_NO_MEM && !p && !test_alloc_live);
    test_alloc_fail=0;
    assert(lcd_ili9881c_create(&cfg,&p)==STM_OK && p && m.commands==0 && m.reset_calls==0);
    lcd_ili9881c_handle_t saved=p; unsigned allocations=test_alloc_calls;
    assert(lcd_ili9881c_create(&cfg,&p)==STM_ERR_INVALID_STATE && p==saved && allocations==test_alloc_calls);
    assert(lcd_ili9881c_display_on_off(p,1)==STM_ERR_INVALID_STATE);
    assert(lcd_ili9881c_display_on_off(p,2)==STM_ERR_INVALID_ARG);
    cfg.io=&m2;
    assert(lcd_ili9881c_create(&cfg,&other)==STM_OK && other!=p && test_alloc_live==2);
    cfg.tx_param=NULL; /* 创建复制配置，后续修改不影响实例。 */
    assert(lcd_ili9881c_reset(p)==STM_OK);
    assert(m.reset_calls==3 && m.delays==3);
    assert(m.reset_values[0]==1 && m.reset_values[1]==0 && m.reset_values[2]==1);
    assert(m.delay_values[0]==5 && m.delay_values[1]==10 && m.delay_values[2]==120);
    m.fail_at=3;
    assert(lcd_ili9881c_init(p)==STM_ERR_TIMEOUT && m.commands==3);
    assert(lcd_ili9881c_display_on_off(p,1)==STM_ERR_INVALID_STATE);
    m.fail_at=0; m.commands=0;
    assert(lcd_ili9881c_init(p)==STM_OK && m.commands>5 && m.last==0x29);
    assert(m2.commands==0 && lcd_ili9881c_display_on_off(other,1)==STM_ERR_INVALID_STATE);
    assert(lcd_ili9881c_init(other)==STM_OK);
    assert(lcd_ili9881c_display_on_off(p,0)==STM_OK && m.last==0x28);
    m.fail_at=m.commands+1;
    assert(lcd_ili9881c_display_on_off(p,1)==STM_ERR_TIMEOUT);
    m.fail_at=0;
    assert(m.first==0xff && m.commands>100);
    m.reset_fail_at=m.reset_calls+2;
    assert(lcd_ili9881c_reset(p)==STM_ERR_IO);
    assert(lcd_ili9881c_display_on_off(p,1)==STM_ERR_INVALID_STATE);
    m.reset_fail_at=0;
    assert(lcd_ili9881c_reset(p)==STM_OK && lcd_ili9881c_init(p)==STM_OK);
    assert(lcd_ili9881c_delete(&p)==STM_OK && !p && test_alloc_live==1);
    assert(lcd_ili9881c_delete(&p)==STM_OK);
    cfg.tx_param=cmd;
    assert(lcd_ili9881c_create(&cfg,&p)==STM_OK);
    assert(lcd_ili9881c_delete(&p)==STM_OK && lcd_ili9881c_delete(&other)==STM_OK && !test_alloc_live);
    return 0;
}
