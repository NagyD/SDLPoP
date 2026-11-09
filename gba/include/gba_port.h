#ifndef GBA_PORT_H
#define GBA_PORT_H
/* Cross-module bindings between SDLPoP, the SDL shim, and hardware support
   code. None of this is exposed to the upstream code — it sees pure SDL. */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Tick / frame timer -------------------------------------------------- */
uint32_t gba_get_ticks_ms(void);
void     gba_delay_ms(uint32_t ms);
void     gba_timer_install(uint32_t interval_ms);
void     gba_timer_uninstall(void);
void     gba_timer_fire_cb(void); /* called by VBlank handler */
void     gba_vblank_isr(void);

/* --- Input --------------------------------------------------------------- */
void gba_input_init(void);
void gba_input_poll(void);
void gba_set_key(int scancode, int down);

/* --- Display: present 240x160 8bpp surface as Mode 4 framebuffer --------- */
void gba_video_init(void);
int  gba_present_surface(SDL_Surface* s);
void gba_apply_palette(const SDL_Color* colors, int first, int n);
void gba_set_viewport(int x, int y); /* offset in pixels into source surface */
void gba_get_viewport(int* x, int* y);
/* When non-zero, gba_camera_update() leaves the viewport untouched so the
   pause/settings menu can pin it (the menu is laid out for the full 320px
   world, wider than the 240px screen; draw_menu() pans the viewport itself). */
extern int gba_menu_active;

/* --- Audio: 8-bit PCM mixer over DirectSound A (DMA1) --------------------- */
void gba_audio_init(const SDL_AudioSpec* desired);
void gba_audio_shutdown(void);
void gba_audio_set_paused(int on);
void gba_audio_lock(void);
void gba_audio_unlock(void);
int  gba_audio_status(void);
void gba_audio_mix_into(int8_t* out, int frames);

/* --- File I/O over linked data blobs ------------------------------------- */
typedef struct gba_blob {
    const char* name;        /* canonical name e.g. "TITLE" or "data/TITLE.DAT" */
    const uint8_t* bytes;
    size_t size;
} gba_blob_t;
const gba_blob_t* gba_find_blob(const char* name);
FILE* gba_fopen(const char* path, const char* mode);
int   gba_access(const char* path);

/* If `fp` is backed by a ROM blob (i.e. it was opened from a linked .bin
   data file), return a direct pointer into ROM at the file's CURRENT position
   plus how many bytes are still readable from that point. Returns NULL if the
   handle is a writable save buffer or unknown.
   The current position is what was set by the last fseek/fread; the caller
   typically does fseek(offset)+fread(checksum) then asks for the pointer. */
const uint8_t* gba_file_rom_ptr(FILE* fp, size_t* out_avail);

/* Heap-safe free: ignores pointers in the ROM address range. SDLPoP's
   load_from_opendats_alloc may now return pointers directly into ROM
   (no copy); calling libc free() on them would corrupt the heap. */
void gba_free_safe(void* p);

/* Static-pool allocator (gba_file.c). The GBA port layer calls these directly
   so its own code uses no malloc/calloc/realloc tokens; upstream src/*.c is
   routed onto the same pool via the linker's --wrap. */
void* gba_alloc(size_t n);
void* gba_calloc(size_t nmemb, size_t size);
void* gba_realloc(void* p, size_t n);
void  gba_free(void* p);

/* Allocate a struct-only SDL_Surface (no pixel buffer); caller points
   ->pixels at sprite data fixed in cartridge ROM. Used by decode_image so no
   throwaway per-sprite pixel buffer is allocated (avoids heap fragmentation). */
struct SDL_Surface;
struct SDL_Surface* gba_new_rom_surface(int w, int h, int bpp);

/* Visual panic: flood the screen with a palette colour and halt (gba_video.c).
   Used by fatal paths so failures show a distinct colour instead of hanging. */
void gba_panic(uint8_t color_idx);

/* Decode a `gba_rom_compressed` SDL_Surface (one whose pixels field is a
   ROM-resident image_data_type pointer rather than a decoded 8bpp buffer)
   into a static scratch buffer, returning a pointer to the 8bpp pixels.
   Layout: w*h contiguous bytes, row stride = w. Caller must NOT free.
   Returns NULL if the source is too large for the scratch.
   Single-threaded use: a second call invalidates the previous result. */
uint8_t* gba_decode_compressed_surface(SDL_Surface* src);

/* --- SRAM save ----------------------------------------------------------- */
#define GBA_SRAM_MAGIC 0x504F5031u  /* 'POP1' */
typedef struct gba_sram_header {
    uint32_t magic;
    uint32_t version;
    uint32_t payload_len;
    uint32_t checksum;
} gba_sram_header_t;

int  gba_sram_write(const void* data, size_t len);
int  gba_sram_read(void* data, size_t cap, size_t* out_len);
void gba_sram_erase(void);

#ifdef __cplusplus
}
#endif
#endif
