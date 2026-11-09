/* PCM audio mixer for GBA.
   - DirectSound A on DMA1, fed by FIFO from a 304-sample ping-pong buffer.
   - Timer 0 reload chosen for ~18157 Hz playback (close to original PoP SFX).
   - Up to MAX_VOICES simultaneous voices, each 8-bit signed PCM.
   - One voice slot is reserved for music streaming (long pre-rendered PCM).
   - The SDL shim's OpenAudio callback is honoured if SDLPoP installs one
     (used by the original midi_callback path on desktop).  On GBA we mostly
     play sounds directly via voice_start() — see gba_sfx_play() below. */
#include <gba.h>
#include <string.h>
#include "gba_port.h"

#define MIX_HZ          18157
#define BUF_FRAMES      304        /* matches ~one VBlank at 18157 Hz (18157/60) */
#define MAX_VOICES      6

typedef struct {
    const int8_t* data;
    uint32_t      length;
    uint32_t      pos;
    uint8_t       volume;       /* 0..255 */
    uint8_t       loop;
    uint8_t       active;
    uint8_t       pad;
} voice_t;

static voice_t s_voices[MAX_VOICES];
static int8_t  s_mix_a[BUF_FRAMES] __attribute__((aligned(4)));
static int8_t  s_mix_b[BUF_FRAMES] __attribute__((aligned(4)));
static int     s_active_buf;
static int     s_paused;
static int     s_locked;
static int     s_initialized;

static SDL_AudioSpec s_spec;

static void start_dma(void) {
    REG_DMA1CNT = 0;
    REG_SOUNDCNT_H = SNDA_VOL_100 | SNDA_R_ENABLE | SNDA_L_ENABLE | SNDA_RESET_FIFO;
    REG_SOUNDCNT_X = SNDSTAT_ENABLE;

    /* Timer 0 reload for 18157 Hz: ((16*1024*1024)/18157) = ~924. */
    REG_TM0CNT_L = 65536 - (16777216 / MIX_HZ);
    REG_TM0CNT_H = TIMER_START;

    /* Kick off DMA1 with the first half. */
    REG_DMA1SAD = (u32)s_mix_a;
    REG_DMA1DAD = (u32)0x040000A0; /* FIFO_A */
    REG_DMA1CNT = DMA_DST_FIXED | DMA_REPEAT | DMA_SRC_INC | DMA32 |
                  DMA_SPECIAL    | DMA_ENABLE | 1;
    s_active_buf = 0;
}

void gba_audio_init(const SDL_AudioSpec* desired) {
    if (desired) s_spec = *desired;
    memset(s_voices, 0, sizeof(s_voices));
    memset(s_mix_a, 0, sizeof(s_mix_a));
    memset(s_mix_b, 0, sizeof(s_mix_b));
    s_paused = 0;
    start_dma();
    s_initialized = 1;
}

void gba_audio_shutdown(void) {
    REG_DMA1CNT = 0;
    REG_SOUNDCNT_X = 0;
}

void gba_audio_set_paused(int on) {
    s_paused = on;
    if (on) memset(s_mix_a, 0, sizeof(s_mix_a)), memset(s_mix_b, 0, sizeof(s_mix_b));
}
void gba_audio_lock(void)   { s_locked = 1; }
void gba_audio_unlock(void) { s_locked = 0; }
int  gba_audio_status(void) { return s_paused ? 0 : 1; }

/* SFX/music API used by sound.c on the GBA target. */
int gba_sfx_play(const int8_t* data, uint32_t length, int loop, uint8_t volume) {
    if (!data || !length) return -1;
    int slot = -1;
    for (int i = 0; i < MAX_VOICES - 1; ++i) {
        if (!s_voices[i].active) { slot = i; break; }
    }
    if (slot < 0) slot = 0; /* steal voice 0 */
    s_voices[slot].data   = data;
    s_voices[slot].length = length;
    s_voices[slot].pos    = 0;
    s_voices[slot].volume = volume;
    s_voices[slot].loop   = (uint8_t)loop;
    s_voices[slot].active = 1;
    return slot;
}
void gba_music_play(const int8_t* data, uint32_t length, int loop) {
    int slot = MAX_VOICES - 1;
    s_voices[slot].data   = data;
    s_voices[slot].length = length;
    s_voices[slot].pos    = 0;
    s_voices[slot].volume = 128;
    s_voices[slot].loop   = (uint8_t)loop;
    s_voices[slot].active = 1;
}
void gba_music_stop(void) {
    s_voices[MAX_VOICES - 1].active = 0;
}
void gba_sfx_stop_all(void) {
    for (int i = 0; i < MAX_VOICES - 1; ++i) s_voices[i].active = 0;
}
int gba_sfx_any_playing(void) {
    for (int i = 0; i < MAX_VOICES; ++i) if (s_voices[i].active) return 1;
    return 0;
}

void gba_audio_mix_into(int8_t* out, int frames) {
    /* Sum into 16-bit accumulator then clip to int8. */
    int16_t acc[BUF_FRAMES];
    memset(acc, 0, sizeof(int16_t) * frames);

    for (int v = 0; v < MAX_VOICES; ++v) {
        voice_t* vp = &s_voices[v];
        if (!vp->active) continue;
        const int8_t* sp = vp->data;
        uint32_t pos = vp->pos, len = vp->length;
        int vol = vp->volume;
        for (int i = 0; i < frames; ++i) {
            if (pos >= len) {
                if (vp->loop) { pos = 0; }
                else { vp->active = 0; break; }
            }
            acc[i] += (int16_t)((int)sp[pos++] * vol >> 8);
        }
        vp->pos = pos;
    }

    for (int i = 0; i < frames; ++i) {
        int16_t v = acc[i];
        if (v > 127)  v = 127;
        if (v < -128) v = -128;
        out[i] = (int8_t)v;
    }
}

/* Called from VBlank ISR — swap the DMA source buffer and refill the idle one. */
void gba_audio_on_vblank(void) {
    /* IRQs are enabled before gba_audio_init() runs (so VBlank-driven SDLPoP
       timing is available immediately). Skip any DMA poking and the 608-byte
       stack-allocated mix buffer until audio is actually set up. */
    if (!s_initialized) return;
    if (s_paused) return;
    if (s_locked) return;

    int8_t* fill_buf;
    if (s_active_buf == 0) {
        /* DMA is on A — refill B and switch source on next wrap */
        fill_buf = s_mix_b;
        gba_audio_mix_into(fill_buf, BUF_FRAMES);
        REG_DMA1CNT = 0;
        REG_DMA1SAD = (u32)s_mix_b;
        REG_DMA1CNT = DMA_DST_FIXED | DMA_REPEAT | DMA_SRC_INC | DMA32 |
                      DMA_SPECIAL    | DMA_ENABLE | 1;
        s_active_buf = 1;
    } else {
        fill_buf = s_mix_a;
        gba_audio_mix_into(fill_buf, BUF_FRAMES);
        REG_DMA1CNT = 0;
        REG_DMA1SAD = (u32)s_mix_a;
        REG_DMA1CNT = DMA_DST_FIXED | DMA_REPEAT | DMA_SRC_INC | DMA32 |
                      DMA_SPECIAL    | DMA_ENABLE | 1;
        s_active_buf = 0;
    }
}
