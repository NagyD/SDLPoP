/* Debug logging for GBA emulators. Emits via BOTH channels so it works on
   whichever the emulator supports:
     1. mGBA / visualboyadvance-m debug register protocol:
          0x04FFF780 (u16) write 0xC0DE to enable (reads back 0x1DEA),
          0x04FFF600 (256B) string buffer,
          0x04FFF700 (u16) write 0x100|level to flush (level 0..4).
     2. AGBPrint (classic VBA): protect 0x09FE2FFE, get 0x09FE20FC,
          put 0x09FE20FE, buffer 0x09FE2000 (2 chars/word), flush = write
          32-bit 0x08FD0000 to 0x09FE20F8. Enable emulator with --agb-print.
   Both are harmless no-ops on real hardware / unsupported emulators. */
#include <gba.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ---- mGBA registers ---- */
#define MGBA_ENABLE (*(volatile unsigned short*)0x04FFF780)
#define MGBA_FLAGS  (*(volatile unsigned short*)0x04FFF700)
#define MGBA_STRING ((volatile char*)0x04FFF600)
static int s_mgba = 0;
volatile int gba_trace = 0;  /* TEMP: gate verbose decode tracing */

/* ---- AGBPrint ---- */
#define AGB_PROTECT (*(volatile unsigned short*)0x09FE2FFE)
#define AGB_GET     (*(volatile unsigned short*)0x09FE20FC)
#define AGB_PUT     (*(volatile unsigned short*)0x09FE20FE)
#define AGB_BUF     ((volatile unsigned short*)0x09FE2000)

int gba_mgba_open(void) {
    MGBA_ENABLE = 0xC0DE;
    s_mgba = (MGBA_ENABLE == 0x1DEA) ? 1 : 0;
    /* reset AGBPrint cursor */
    AGB_PROTECT = 0x20;
    AGB_GET = 0;
    AGB_PUT = 0;
    AGB_PROTECT = 0;
    return s_mgba;
}

static void agb_putc(char c) {
    AGB_PROTECT = 0x20;
    unsigned short put = AGB_PUT;
    volatile unsigned short* w = &AGB_BUF[put >> 1];
    unsigned short v = *w;
    if (put & 1) v = (v & 0x00FF) | ((unsigned short)(unsigned char)c << 8);
    else         v = (v & 0xFF00) | (unsigned char)c;
    *w = v;
    AGB_PUT = (unsigned short)(put + 1);
    AGB_PROTECT = 0;
}

void gba_log(const char* fmt, ...) {
    char buf[200];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    int n = (int)strlen(buf);
    if (n > 255) n = 255;

    /* mGBA: copy + flush at level 3 (info) */
    for (int i = 0; i < n; ++i) MGBA_STRING[i] = buf[i];
    MGBA_STRING[n] = 0;
    MGBA_FLAGS = 0x100 | 3;

    /* AGBPrint: stream chars + flush */
    for (int i = 0; i < n; ++i) agb_putc(buf[i]);
    agb_putc('\n');
    *(volatile unsigned*)0x09FE20F8 = 0x08FD0000u;
    AGB_PROTECT = 0x20; AGB_GET = 0; AGB_PUT = 0; AGB_PROTECT = 0;
}
