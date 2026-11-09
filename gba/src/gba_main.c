/* GBA entry point. Replaces src/main.c (which is excluded by the Makefile). */
#include <gba.h>
#include <stdio.h>
#include <locale.h>
#include "common.h"
#include "gba_port.h"

extern void pop_main(void);
extern void gba_input_init(void);
extern void gba_video_init(void);
extern void gba_vblank_isr(void);

/* g_argc / g_argv are defined upstream in data.c via the BODY/data.h
   mechanism — do NOT redeclare them here. We just point them at a dummy
   argv before pop_main(). */
extern int    g_argc;
extern char** g_argv;
static char*  s_argv0[] = { (char*)"sdlpop" };

/* GamePak wait-state control register (libgba doesn't define it). */
#define REG_WAITCNT_ (*(volatile unsigned short*)0x04000204)

int main(void) {
    /* Enable GamePak prefetch + fastest wait states (WS0 3,1 / prefetch on).
       The BIOS leaves this at 0 (slowest: 4,2 cycles, no prefetch), which
       roughly doubles the cost of every instruction fetched from cartridge
       ROM — and all the hot blit/composite/decode loops run from ROM. This
       is the cheapest, largest single speedup for the port. */
    REG_WAITCNT_ = 0x4317;

    /* Force the single-byte "C" locale. This newlib is built multibyte-capable
       and was defaulting to a UTF-8 locale, so *printf-to-string (vsnprintf et
       al.) scanned the format string through __utf8_mbtowc — which spun
       forever, freezing the game the moment gba_log() ran on the first move.
       MB_CUR_MAX==1 makes vfprintf take the fast single-byte path. */
    setlocale(LC_ALL, "C");

    g_argc = 1;
    g_argv = s_argv0;

    /* Enable AGBPrint / mGBA debug logging (visible in VBA-M / mGBA when run
       with --agb-print). Harmless on real hardware. gba_log() is available
       from gba_debug.c for future debugging. */
    extern int gba_mgba_open(void);
    gba_mgba_open();

    gba_video_init();
    gba_input_init();

    irqInit();
    irqSet(IRQ_VBLANK, gba_vblank_isr);
    irqEnable(IRQ_VBLANK);

    pop_main();

    while (1) VBlankIntrWait();
    return 0;
}
