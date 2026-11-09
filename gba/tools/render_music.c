/* render_music — host-side MIDI -> 8-bit PCM renderer for the GBA port.
 *
 * The GBA build can't run the OPL3 FM synth live, and PoP's music exists
 * only as MIDI (in MIDISND1.DAT / MIDISND2.DAT).  This tool reuses the exact
 * desktop synthesis code (src/midi.c + src/opl3.c) to render each music track
 * to signed 8-bit, mono, 18157 Hz PCM — the format the GBA DMA mixer consumes
 * (see gba/src/gba_audio.c) — and packs them into a single blob, gba/data/
 * musicpcm.bin, which the Makefile embeds in ROM via bin2s.
 *
 * Layout of musicpcm.bin (all integers little-endian; GBA is LE):
 *     u32 magic   = 'M''P''C''M'  (0x4D43504D)
 *     u32 count
 *     count x { u32 sound_id; u32 offset; u32 length }   // offset from blob base
 *     ... concatenated PCM payloads ...
 *
 * Usage: render_music <data-dir> <out.bin>
 *   e.g. render_music data data/musicpcm.bin   (run from gba/)
 *
 * Builds with any host C compiler + SDL2 *headers* (no SDL2 lib needed):
 *   cc -O2 $(sdl2-config --cflags) -I../src -o render_music \
 *      render_music.c ../../src/midi.c ../../src/opl3.c -lm
 */
#define SDL_MAIN_HANDLED 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"   /* types.h (sound_buffer_type, dat_*), proto.h, data.h */

#define TARGET_HZ      18157   /* must match MIX_HZ in gba/src/gba_audio.c */
#define MAX_SOUND_ID   58
#define MAX_SECONDS    90      /* safety cap per track */
#define TARGET_PEAK    120     /* normalize each track to this 8-bit peak */
#define MUSICPCM_MAGIC 0x4D43504Du  /* 'M','P','C','M' */

/* ---- globals that src/midi.c expects (declared extern in data.h / midi.c) -- */
short          midi_playing    = 0;
static SDL_AudioSpec g_spec;
SDL_AudioSpec* digi_audiospec  = &g_spec;
int            digi_unavailable = 0;
byte           is_sound_on     = 1;
byte           enable_music    = 1;
word           current_sound   = 0;

/* ---- SDL stubs (we link the headers only, not the SDL2 library) ----------- */
void SDL_LockAudio(void)        {}
void SDL_UnlockAudio(void)      {}
void SDL_PauseAudio(int p)      { (void)p; }
#undef SDL_memset
void* SDL_memset(void* dst, int c, size_t len) { return memset(dst, c, len); }
void quit(int code)             { exit(code); }
void init_digi(void)            { /* spec is configured in main() */ }

/* --- Minimal DAT chain, mirroring src/seg009.c open_dat/load_from_opendats - */
typedef struct host_dat {
    struct host_dat* next;
    FILE*  fp;
    byte*  table;      /* res_count (u16) + entries[] (id u16, off u32, size u16) */
    int    table_size;
} host_dat;

static host_dat* g_chain = NULL;
static const char* g_datadir = ".";

dat_type* open_dat(const char* filename, int optional) {
    (void)optional;
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", g_datadir, filename);
    FILE* fp = fopen(path, "rb");
    host_dat* d = (host_dat*)calloc(1, sizeof(host_dat));
    d->next = g_chain;
    g_chain = d;
    if (fp) {
        dat_header_type hdr;
        if (fread(&hdr, 6, 1, fp) == 1) {
            d->table_size = (int)SDL_SwapLE16(hdr.table_size);
            d->table = (byte*)malloc(d->table_size);
            if (d->table &&
                fseek(fp, (long)SDL_SwapLE32(hdr.table_offset), SEEK_SET) == 0 &&
                fread(d->table, d->table_size, 1, fp) == 1) {
                d->fp = fp;
            } else {
                free(d->table); d->table = NULL;
                fclose(fp);
            }
        } else {
            fclose(fp);
        }
    }
    /* We return an opaque non-NULL token; midi.c only passes it to close_dat. */
    return (dat_type*)d;
}

void close_dat(dat_type* pointer) {
    host_dat** prev = &g_chain;
    for (host_dat* c = g_chain; c; prev = &c->next, c = c->next) {
        if ((dat_type*)c == pointer) {
            *prev = c->next;
            if (c->fp) fclose(c->fp);
            free(c->table);
            free(c);
            return;
        }
    }
}

void* load_from_opendats_alloc(int resource, const char* extension,
                               data_location* out_result, int* out_size) {
    (void)extension;
    if (out_result) *out_result = data_none;
    if (out_size) *out_size = 0;
    for (host_dat* d = g_chain; d; d = d->next) {
        if (!d->fp || !d->table) continue;
        word res_count = SDL_SwapLE16(*(word*)d->table);
        const byte* e = d->table + 2;
        for (int i = 0; i < res_count; ++i, e += 8) {
            word id = SDL_SwapLE16(*(word*)e);
            if (id != resource) continue;
            dword off = SDL_SwapLE32(*(dword*)(e + 2));
            word size = SDL_SwapLE16(*(word*)(e + 6));
            byte chk;
            if (fseek(d->fp, (long)off, SEEK_SET) || fread(&chk, 1, 1, d->fp) != 1)
                return NULL;
            void* area = malloc(size);
            if (!area) return NULL;
            if (size && fread(area, size, 1, d->fp) != 1) { free(area); return NULL; }
            if (out_result) *out_result = data_DAT;
            if (out_size) *out_size = size;
            return area;
        }
    }
    return NULL;
}

/* --- PCM accumulation ------------------------------------------------------ */
typedef struct { byte* data; size_t len, cap; } buf_t;
static void buf_put(buf_t* b, byte v) {
    if (b->len == b->cap) { b->cap = b->cap ? b->cap * 2 : 65536; b->data = realloc(b->data, b->cap); }
    b->data[b->len++] = v;
}

/* Render the currently-loaded MIDI (midi_playing must be 1) to mono int8 PCM.
   Returns malloc'd signed-8-bit samples in *out_pcm / *out_len (caller frees). */
static void render_current_track(int8_t** out_pcm, int* out_len) {
    enum { CHUNK = 4096 };
    short stereo[CHUNK * 2];
    short* mono16 = NULL; size_t mono_len = 0, mono_cap = 0;
    long max_frames = (long)TARGET_HZ * MAX_SECONDS;
    long produced = 0;
    int peak = 1;

    while (midi_playing && produced < max_frames) {
        memset(stereo, 0, sizeof(stereo));
        midi_callback(NULL, (Uint8*)stereo, CHUNK * 4);
        for (int i = 0; i < CHUNK; ++i) {
            int m = (stereo[i * 2] + stereo[i * 2 + 1]) / 2;
            if (mono_len == mono_cap) {
                mono_cap = mono_cap ? mono_cap * 2 : (CHUNK * 8);
                mono16 = realloc(mono16, mono_cap * sizeof(short));
            }
            mono16[mono_len++] = (short)m;
            int a = m < 0 ? -m : m;
            if (a > peak) peak = a;
        }
        produced += CHUNK;
    }
    /* Normalize peak -> TARGET_PEAK, convert to signed 8-bit. */
    int8_t* pcm = malloc(mono_len ? mono_len : 1);
    for (size_t i = 0; i < mono_len; ++i) {
        int v = (mono16[i] * TARGET_PEAK) / peak;
        if (v > 127) v = 127; if (v < -128) v = -128;
        pcm[i] = (int8_t)v;
    }
    /* Trim trailing silence. */
    size_t n = mono_len;
    while (n > 0 && pcm[n - 1] == 0) --n;
    free(mono16);
    *out_pcm = pcm;
    *out_len = (int)n;
}

static void put_u32(buf_t* b, dword v) {
    buf_put(b, v & 0xFF); buf_put(b, (v >> 8) & 0xFF);
    buf_put(b, (v >> 16) & 0xFF); buf_put(b, (v >> 24) & 0xFF);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <data-dir> <out.bin>\n", argv[0]);
        return 2;
    }
    g_datadir = argv[1];

    g_spec.freq = TARGET_HZ;
    g_spec.format = AUDIO_S8;
    g_spec.channels = 1;
    g_spec.samples = 304;

    /* Keep the MIDI DATs open for the whole run; init_midi() handles PRINCE.DAT
       (instruments) itself.  load_from_opendats_alloc searches the chain. */
    open_dat("MIDISND1.DAT", 0);
    open_dat("MIDISND2.DAT", 0);

    /* Collected results. */
    int    ids[MAX_SOUND_ID];
    int8_t* pcms[MAX_SOUND_ID];
    int    lens[MAX_SOUND_ID];
    int    count = 0;

    for (int index = 0; index < MAX_SOUND_ID; ++index) {
        int size = 0;
        sound_buffer_type* buffer =
            (sound_buffer_type*)load_from_opendats_alloc(index + 10000, "bin", NULL, &size);
        if (!buffer) continue;
        if ((buffer->type & 7) != sound_midi) { free(buffer); continue; }

        current_sound = (word)index;
        play_midi_sound(buffer);   /* parses MIDI, sets midi_playing = 1 */
        if (!midi_playing) { free(buffer); continue; }

        int8_t* pcm; int len;
        render_current_track(&pcm, &len);
        free(buffer);
        if (len <= 0) { free(pcm); continue; }

        ids[count] = index; pcms[count] = pcm; lens[count] = len;
        fprintf(stderr, "  track %2d -> %d samples (%.2fs)\n",
                index, len, (double)len / TARGET_HZ);
        ++count;
    }

    /* Pack the blob. */
    buf_t out = {0};
    put_u32(&out, MUSICPCM_MAGIC);
    put_u32(&out, (dword)count);
    dword data_base = 8 + (dword)count * 12;
    dword cursor = data_base;
    for (int i = 0; i < count; ++i) {
        put_u32(&out, (dword)ids[i]);
        put_u32(&out, cursor);
        put_u32(&out, (dword)lens[i]);
        cursor += (dword)lens[i];
    }
    for (int i = 0; i < count; ++i)
        for (int j = 0; j < lens[i]; ++j)
            buf_put(&out, (byte)pcms[i][j]);

    FILE* f = fopen(argv[2], "wb");
    if (!f) { perror(argv[2]); return 1; }
    fwrite(out.data, 1, out.len, f);
    fclose(f);
    fprintf(stderr, "%s: %d music tracks, %zu bytes\n", argv[2], count, out.len);
    return 0;
}
