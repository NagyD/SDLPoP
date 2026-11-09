/* Tick counter and VBlank wait. Timer 0 increments at 16384 Hz so a 32-bit
   millisecond counter is enough for ~49 days. */
#include <gba.h>
#include "gba_port.h"

static volatile uint32_t s_vblank_count;
static uint32_t s_timer_interval_ms;
static uint32_t s_timer_next_fire;

void gba_vblank_isr(void) {
    s_vblank_count++;
    /* Mix one VBlank's worth of audio (the audio buffer is sized to match). */
    extern void gba_audio_on_vblank(void);
    gba_audio_on_vblank();

    if (s_timer_interval_ms && s_vblank_count >= s_timer_next_fire) {
        s_timer_next_fire = s_vblank_count + (s_timer_interval_ms * 60 + 999) / 1000;
        gba_timer_fire_cb();
    }
}

uint32_t gba_get_ticks_ms(void) {
    /* 60 Hz VBlank: ms = count * 1000 / 60. Use 32-bit arithmetic carefully. */
    uint32_t c = s_vblank_count;
    return (c * 1000u + 30u) / 60u;
}

void gba_delay_ms(uint32_t ms) {
    uint32_t target = s_vblank_count + (ms * 60 + 999) / 1000;
    while (s_vblank_count < target) VBlankIntrWait();
}

void gba_timer_install(uint32_t interval_ms) {
    s_timer_interval_ms = interval_ms;
    s_timer_next_fire   = s_vblank_count + (interval_ms * 60 + 999) / 1000;
}
void gba_timer_uninstall(void) {
    s_timer_interval_ms = 0;
}
