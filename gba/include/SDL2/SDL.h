/* Minimal SDL2 surface/event/audio/timer shim for GBA (devkitARM).
   Only the subset actually exercised by SDLPoP is implemented.
   This header is included instead of the real SDL2/SDL.h when building for GBA. */
#ifndef SDL_GBA_SHIM_H
#define SDL_GBA_SHIM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t   Sint8;
typedef uint8_t  Uint8;
typedef int16_t  Sint16;
typedef uint16_t Uint16;
typedef int32_t  Sint32;
typedef uint32_t Uint32;
typedef int64_t  Sint64;
typedef uint64_t Uint64;
typedef Uint8    SDL_bool;

#define SDL_TRUE  1
#define SDL_FALSE 0

#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321
#define SDL_BYTEORDER  SDL_LIL_ENDIAN

#define SDL_INIT_VIDEO        0x00000020u
#define SDL_INIT_AUDIO        0x00000010u
#define SDL_INIT_JOYSTICK     0x00000200u
#define SDL_INIT_GAMECONTROLLER 0x00002000u
#define SDL_INIT_TIMER        0x00000001u
#define SDL_INIT_HAPTIC       0x00001000u
#define SDL_INIT_EVERYTHING   0x0000FFFFu
#define SDL_INIT_NOPARACHUTE  0x00100000u

#define SDL_HINT_RENDER_VSYNC          "SDL_RENDER_VSYNC"
#define SDL_HINT_RENDER_SCALE_QUALITY  "SDL_RENDER_SCALE_QUALITY"

#define SDL_ALPHA_OPAQUE      255
#define SDL_ALPHA_TRANSPARENT 0
#define SDL_SRCCOLORKEY       0x00001000

typedef struct SDL_Color {
    Uint8 r, g, b, a;
} SDL_Color;

/* PoP sprites are 4bpp (16 colours/sprite). The screen surface is 8bpp but its
   palette is never populated (we use the global PoP palette[] for hardware
   BG_PAL). Cutting from 256 colours inline to 16 saves ~960 bytes per surface
   instance — multiplied by ~100+ surfaces (font + sprites), this is the
   difference between fitting on the GBA's 256 KB EWRAM or not. */
#define SDL_PALETTE_SIZE 16
typedef struct SDL_Palette {
    int       ncolors;
    SDL_Color colors[SDL_PALETTE_SIZE];
} SDL_Palette;

typedef struct SDL_PixelFormat {
    Uint32       format;
    SDL_Palette* palette;
    Uint8        BitsPerPixel;
    Uint8        BytesPerPixel;
    Uint32       Rmask, Gmask, Bmask, Amask;
} SDL_PixelFormat;

typedef struct SDL_Rect {
    int x, y, w, h;
} SDL_Rect;

typedef enum {
    SDL_BLENDMODE_NONE  = 0,
    SDL_BLENDMODE_BLEND = 1,
    SDL_BLENDMODE_ADD   = 2,
    SDL_BLENDMODE_MOD   = 3
} SDL_BlendMode;

typedef struct SDL_Surface {
    Uint32           flags;
    SDL_PixelFormat* format;
    int              w, h;
    int              pitch;
    void*            pixels;
    SDL_Rect         clip_rect;
    int              refcount;

    /* Internal: palette/format storage attached to the surface. */
    SDL_PixelFormat  _format;
    SDL_Palette      _palette;
    int              has_colorkey;
    Uint32           colorkey;
    Uint8            alpha_mod;
    SDL_BlendMode    blend_mode;

    /* GBA: sprite stored as pointer-to-ROM image_data_type instead of a
       decoded 8bpp pixel buffer. Set by decode_image; consumed by
       SDL_BlitSurface, which decompresses on the fly into a scratch. */
    Uint8            gba_rom_compressed;  /* 1 if pixels points to ROM */
    Uint8            gba_palette_offset;  /* row*16 baked at blit time */
    Uint8            gba_depth;           /* bpp from DAT header */
    Uint8            gba_cmeth;           /* compression method 0..4 */
} SDL_Surface;

typedef struct SDL_Window   { int _; } SDL_Window;
typedef struct SDL_Renderer { int _; } SDL_Renderer;
typedef struct SDL_Texture  { int _; } SDL_Texture;
typedef struct SDL_RWops       SDL_RWops;
typedef struct SDL_GameController { int _; } SDL_GameController;
typedef struct SDL_Joystick { int _; } SDL_Joystick;
typedef struct SDL_Haptic   { int _; } SDL_Haptic;
typedef int SDL_TimerID;

/* Pixel formats we report */
#define SDL_PIXELFORMAT_INDEX8 0x13000801u
#define SDL_PIXELFORMAT_RGB24  0x17101803u
#define SDL_PIXELFORMAT_ARGB8888 0x16362004u

#define SDL_ISPIXELFORMAT_INDEXED(f) ((f) == SDL_PIXELFORMAT_INDEX8)

/* Endian swap (we're always little endian on GBA/ARM7). */
static inline Uint16 SDL_Swap16(Uint16 x) { return (Uint16)((x<<8)|(x>>8)); }
static inline Uint32 SDL_Swap32(Uint32 x) {
    return ((x<<24)&0xff000000u)|((x<<8)&0x00ff0000u)|
           ((x>>8)&0x0000ff00u)|((x>>24)&0x000000ffu);
}
#define SDL_SwapLE16(x) ((Uint16)(x))
#define SDL_SwapLE32(x) ((Uint32)(x))
#define SDL_SwapBE16(x) SDL_Swap16(x)
#define SDL_SwapBE32(x) SDL_Swap32(x)

/* Lifecycle */
int  SDL_Init(Uint32 flags);
int  SDL_InitSubSystem(Uint32 flags);
void SDL_Quit(void);
void SDL_SetHint(const char* name, const char* value);
const char* SDL_GetError(void);
void SDL_SetError(const char* fmt, ...);

/* Surfaces */
SDL_Surface* SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth,
                                  Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
void SDL_FreeSurface(SDL_Surface* s);
int  SDL_LockSurface(SDL_Surface* s);
void SDL_UnlockSurface(SDL_Surface* s);
int  SDL_FillRect(SDL_Surface* dst, const SDL_Rect* rect, Uint32 color);
int  SDL_BlitSurface(SDL_Surface* src, const SDL_Rect* srcrect,
                     SDL_Surface* dst, SDL_Rect* dstrect);
#define SDL_UpperBlit SDL_BlitSurface
int  SDL_BlitScaled(SDL_Surface* src, const SDL_Rect* srcrect,
                    SDL_Surface* dst, SDL_Rect* dstrect);
int  SDL_SetColorKey(SDL_Surface* s, int flag, Uint32 key);
int  SDL_SetSurfaceAlphaMod(SDL_Surface* s, Uint8 alpha);
int  SDL_SetSurfaceBlendMode(SDL_Surface* s, SDL_BlendMode mode);
int  SDL_SetClipRect(SDL_Surface* s, const SDL_Rect* rect);
int  SDL_SetPaletteColors(SDL_Palette* palette, const SDL_Color* colors,
                          int firstcolor, int ncolors);
SDL_Surface* SDL_ConvertSurfaceFormat(SDL_Surface* src, Uint32 fmt, Uint32 flags);
SDL_Surface* SDL_ConvertSurface(SDL_Surface* src, const SDL_PixelFormat* fmt, Uint32 flags);
int          SDL_SetSurfacePalette(SDL_Surface* s, SDL_Palette* palette);
Uint32 SDL_MapRGB(const SDL_PixelFormat* fmt, Uint8 r, Uint8 g, Uint8 b);
Uint32 SDL_MapRGBA(const SDL_PixelFormat* fmt, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
const char* SDL_GetPixelFormatName(Uint32 fmt);

/* RWops (only memory-backed flavor is implemented) */
struct SDL_RWops {
    Sint64 (*size)(SDL_RWops*);
    Sint64 (*seek)(SDL_RWops*, Sint64, int);
    size_t (*read)(SDL_RWops*, void*, size_t, size_t);
    size_t (*write)(SDL_RWops*, const void*, size_t, size_t);
    int    (*close)(SDL_RWops*);
    Uint32 type;
    union {
        struct { const Uint8* base; const Uint8* here; const Uint8* stop; } mem;
    } hidden;
};
#define RW_SEEK_SET 0
#define RW_SEEK_CUR 1
#define RW_SEEK_END 2
SDL_RWops* SDL_RWFromConstMem(const void* mem, int size);
SDL_RWops* SDL_RWFromMem(void* mem, int size);
SDL_RWops* SDL_RWFromFile(const char* file, const char* mode);
int        SDL_RWclose(SDL_RWops* rw);
size_t     SDL_RWread(SDL_RWops* rw, void* ptr, size_t size, size_t maxnum);
size_t     SDL_RWwrite(SDL_RWops* rw, const void* ptr, size_t size, size_t num);
Sint64     SDL_RWseek(SDL_RWops* rw, Sint64 ofs, int whence);
Sint64     SDL_RWtell(SDL_RWops* rw);
Sint64     SDL_RWsize(SDL_RWops* rw);

/* Timing */
Uint32 SDL_GetTicks(void);
Uint64 SDL_GetPerformanceCounter(void);
Uint64 SDL_GetPerformanceFrequency(void);
void   SDL_Delay(Uint32 ms);

/* Events / input */
#define SDL_NUM_SCANCODES 256
typedef int SDL_Scancode;
enum {
    SDL_SCANCODE_UNKNOWN = 0,
    SDL_SCANCODE_A = 4,  SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D,
    SDL_SCANCODE_E, SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H,
    SDL_SCANCODE_I, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L,
    SDL_SCANCODE_M, SDL_SCANCODE_N, SDL_SCANCODE_O, SDL_SCANCODE_P,
    SDL_SCANCODE_Q, SDL_SCANCODE_R, SDL_SCANCODE_S, SDL_SCANCODE_T,
    SDL_SCANCODE_U, SDL_SCANCODE_V, SDL_SCANCODE_W, SDL_SCANCODE_X,
    SDL_SCANCODE_Y, SDL_SCANCODE_Z,
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8,
    SDL_SCANCODE_9, SDL_SCANCODE_0,
    SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE, SDL_SCANCODE_BACKSPACE,
    SDL_SCANCODE_TAB, SDL_SCANCODE_SPACE,
    SDL_SCANCODE_MINUS, SDL_SCANCODE_EQUALS,
    SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET, SDL_SCANCODE_BACKSLASH,
    SDL_SCANCODE_NONUSHASH, SDL_SCANCODE_SEMICOLON, SDL_SCANCODE_APOSTROPHE,
    SDL_SCANCODE_GRAVE, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD, SDL_SCANCODE_SLASH,
    SDL_SCANCODE_CAPSLOCK,
    SDL_SCANCODE_F1, SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4,
    SDL_SCANCODE_F5, SDL_SCANCODE_F6, SDL_SCANCODE_F7, SDL_SCANCODE_F8,
    SDL_SCANCODE_F9, SDL_SCANCODE_F10, SDL_SCANCODE_F11, SDL_SCANCODE_F12,
    SDL_SCANCODE_PRINTSCREEN, SDL_SCANCODE_SCROLLLOCK, SDL_SCANCODE_PAUSE,
    SDL_SCANCODE_INSERT, SDL_SCANCODE_HOME, SDL_SCANCODE_PAGEUP,
    SDL_SCANCODE_DELETE, SDL_SCANCODE_END, SDL_SCANCODE_PAGEDOWN,
    SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_DOWN, SDL_SCANCODE_UP,
    SDL_SCANCODE_NUMLOCKCLEAR,
    SDL_SCANCODE_KP_DIVIDE, SDL_SCANCODE_KP_MULTIPLY, SDL_SCANCODE_KP_MINUS,
    SDL_SCANCODE_KP_PLUS, SDL_SCANCODE_KP_ENTER,
    SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3, SDL_SCANCODE_KP_4,
    SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_8,
    SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0,
    SDL_SCANCODE_KP_PERIOD, SDL_SCANCODE_NONUSBACKSLASH, SDL_SCANCODE_APPLICATION,
    SDL_SCANCODE_POWER, SDL_SCANCODE_KP_EQUALS,
    SDL_SCANCODE_F13, SDL_SCANCODE_F14, SDL_SCANCODE_F15, SDL_SCANCODE_F16,
    SDL_SCANCODE_F17, SDL_SCANCODE_F18, SDL_SCANCODE_F19, SDL_SCANCODE_F20,
    SDL_SCANCODE_F21, SDL_SCANCODE_F22, SDL_SCANCODE_F23, SDL_SCANCODE_F24,
    SDL_SCANCODE_EXECUTE, SDL_SCANCODE_HELP, SDL_SCANCODE_MENU, SDL_SCANCODE_SELECT,
    SDL_SCANCODE_STOP, SDL_SCANCODE_AGAIN, SDL_SCANCODE_UNDO, SDL_SCANCODE_CUT,
    SDL_SCANCODE_COPY, SDL_SCANCODE_PASTE, SDL_SCANCODE_FIND, SDL_SCANCODE_MUTE,
    SDL_SCANCODE_VOLUMEUP, SDL_SCANCODE_VOLUMEDOWN,
    SDL_SCANCODE_LCTRL = 224, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_LALT, SDL_SCANCODE_LGUI,
    SDL_SCANCODE_RCTRL,        SDL_SCANCODE_RSHIFT, SDL_SCANCODE_RALT, SDL_SCANCODE_RGUI,
    SDL_SCANCODE_CLEAR = 156,
    SDL_SCANCODE_AUDIOMUTE = 240,
};
typedef int SDL_Keycode;

typedef enum {
    SDL_FIRSTEVENT     = 0,
    SDL_QUIT           = 0x100,
    SDL_KEYDOWN        = 0x300,
    SDL_KEYUP          = 0x301,
    SDL_TEXTEDITING    = 0x302,
    SDL_TEXTINPUT      = 0x303,
    SDL_MOUSEMOTION    = 0x400,
    SDL_MOUSEBUTTONDOWN= 0x401,
    SDL_MOUSEBUTTONUP  = 0x402,
    SDL_MOUSEWHEEL     = 0x403,
    SDL_JOYAXISMOTION  = 0x600,
    SDL_JOYBUTTONDOWN  = 0x603,
    SDL_JOYBUTTONUP    = 0x604,
    SDL_CONTROLLERAXISMOTION = 0x650,
    SDL_CONTROLLERBUTTONDOWN = 0x651,
    SDL_CONTROLLERBUTTONUP   = 0x652,
    SDL_CONTROLLERDEVICEADDED= 0x653,
    SDL_CONTROLLERDEVICEREMOVED= 0x654,
    SDL_WINDOWEVENT    = 0x200,
    SDL_USEREVENT      = 0x8000,
} SDL_EventType;

typedef struct { Uint32 type; Uint32 timestamp; } SDL_CommonEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; Uint32 windowID;
    Uint8 state; Uint8 repeat; Uint8 padding2, padding3;
    struct { SDL_Scancode scancode; SDL_Keycode sym; Uint16 mod; Uint32 unused; } keysym;
} SDL_KeyboardEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; Uint32 windowID; char text[32];
} SDL_TextInputEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; Uint32 windowID;
    Uint8 event; Uint8 padding1, padding2, padding3;
    Sint32 data1, data2;
} SDL_WindowEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; int which;
    Uint8 button; Uint8 state; Uint8 padding1, padding2;
    Sint32 x, y;
} SDL_MouseButtonEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; int which; Sint32 x, y;
    Uint32 direction;
} SDL_MouseWheelEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; int which;
    Uint8  axis; Uint8 padding1, padding2, padding3;
    Sint16 value; Uint16 padding4;
} SDL_ControllerAxisEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; int which;
    Uint8 button; Uint8 state; Uint8 padding1, padding2;
} SDL_ControllerButtonEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; int which;
} SDL_ControllerDeviceEvent;
typedef struct {
    Uint32 type; Uint32 timestamp; Uint32 windowID;
    Sint32 code; void* data1; void* data2;
} SDL_UserEvent;
enum {
    KMOD_NONE   = 0,
    KMOD_LSHIFT = 0x0001, KMOD_RSHIFT = 0x0002,
    KMOD_LCTRL  = 0x0040, KMOD_RCTRL  = 0x0080,
    KMOD_LALT   = 0x0100, KMOD_RALT   = 0x0200,
    KMOD_LGUI   = 0x0400, KMOD_RGUI   = 0x0800,
    KMOD_NUM    = 0x1000, KMOD_CAPS   = 0x2000,
    KMOD_SHIFT  = KMOD_LSHIFT | KMOD_RSHIFT,
    KMOD_CTRL   = KMOD_LCTRL  | KMOD_RCTRL,
    KMOD_ALT    = KMOD_LALT   | KMOD_RALT,
    KMOD_GUI    = KMOD_LGUI   | KMOD_RGUI,
};
typedef SDL_ControllerAxisEvent SDL_JoyAxisEvent;
typedef SDL_ControllerButtonEvent SDL_JoyButtonEvent;

typedef union SDL_Event {
    Uint32 type;
    SDL_CommonEvent common;
    SDL_KeyboardEvent key;
    SDL_TextInputEvent text;
    SDL_WindowEvent window;
    SDL_MouseButtonEvent button;
    SDL_MouseWheelEvent wheel;
    SDL_ControllerAxisEvent caxis;
    SDL_ControllerButtonEvent cbutton;
    SDL_ControllerDeviceEvent cdevice;
    SDL_JoyAxisEvent jaxis;
    SDL_JoyButtonEvent jbutton;
    SDL_UserEvent user;
    Uint8 padding[56];
} SDL_Event;

int  SDL_PollEvent(SDL_Event* event);
int  SDL_PushEvent(SDL_Event* event);
const Uint8* SDL_GetKeyboardState(int* numkeys);
Uint32 SDL_GetMouseState(int* x, int* y);
const char* SDL_GetScancodeName(SDL_Scancode scancode);
int  SDL_ShowCursor(int toggle);
void SDL_StartTextInput(void);
void SDL_StopTextInput(void);
int  SDL_SetTextInputRect(const SDL_Rect* rect);
#define SDL_DISABLE 0
#define SDL_ENABLE  1

/* Window/renderer/texture: kept as opaque stubs (we render directly to GBA VRAM). */
#define SDL_WINDOWPOS_UNDEFINED 0x1FFF0000u
#define SDL_WINDOW_RESIZABLE        0x00000020u
#define SDL_WINDOW_ALLOW_HIGHDPI    0x00002000u
#define SDL_WINDOW_FULLSCREEN_DESKTOP 0x00001001u
#define SDL_RENDERER_SOFTWARE       0x00000001u
#define SDL_RENDERER_ACCELERATED    0x00000002u
#define SDL_RENDERER_TARGETTEXTURE  0x00000008u
#define SDL_TEXTUREACCESS_TARGET    2
#define SDL_TEXTUREACCESS_STREAMING 1

SDL_Window*   SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
void          SDL_DestroyWindow(SDL_Window* w);
SDL_Renderer* SDL_CreateRenderer(SDL_Window* w, int index, Uint32 flags);
void          SDL_DestroyRenderer(SDL_Renderer* r);
SDL_Texture*  SDL_CreateTexture(SDL_Renderer* r, Uint32 fmt, int access, int w, int h);
void          SDL_DestroyTexture(SDL_Texture* t);
int  SDL_RenderClear(SDL_Renderer* r);
int  SDL_RenderCopy(SDL_Renderer* r, SDL_Texture* t, const SDL_Rect* src, const SDL_Rect* dst);
void SDL_RenderPresent(SDL_Renderer* r);
int  SDL_SetRenderTarget(SDL_Renderer* r, SDL_Texture* t);
int  SDL_UpdateTexture(SDL_Texture* t, const SDL_Rect* rect, const void* px, int pitch);
int  SDL_RenderSetLogicalSize(SDL_Renderer* r, int w, int h);
int  SDL_RenderSetIntegerScale(SDL_Renderer* r, SDL_bool enable);
int  SDL_RenderGetLogicalSize(SDL_Renderer* r, int* w, int* h);
void SDL_RenderGetScale(SDL_Renderer* r, float* scaleX, float* scaleY);
void SDL_RenderGetViewport(SDL_Renderer* r, SDL_Rect* rect);
int  SDL_GetRendererOutputSize(SDL_Renderer* r, int* w, int* h);
int  SDL_GetWindowFlags(SDL_Window* w);
void SDL_SetWindowFullscreen(SDL_Window* w, Uint32 flags);
void SDL_SetWindowIcon(SDL_Window* w, SDL_Surface* icon);
typedef struct { const char* name; Uint32 flags; Uint32 num_texture_formats; Uint32 texture_formats[16]; int max_texture_width; int max_texture_height; } SDL_RendererInfo;
int  SDL_GetRendererInfo(SDL_Renderer* r, SDL_RendererInfo* info);
void SDL_GL_GetDrawableSize(SDL_Window* w, int* a, int* b);
void SDL_GetWindowSize(SDL_Window* w, int* a, int* b);
int  SDL_UpdateRect(SDL_Surface* s, int x, int y, int w, int h);
int  SDL_Flip(SDL_Surface* s);

/* Audio (PCM via DMA1; AudioCVT is no-op as we feed 8-bit signed PCM already) */
typedef struct {
    int freq; Uint16 format; Uint8 channels; Uint8 silence;
    Uint16 samples; Uint16 padding; Uint32 size;
    void (*callback)(void* userdata, Uint8* stream, int len);
    void* userdata;
} SDL_AudioSpec;
#define AUDIO_S8     0x8008
#define AUDIO_U8     0x0008
#define AUDIO_S16SYS 0x8010
#define AUDIO_S16LSB 0x8010
typedef Uint16 SDL_AudioFormat;
typedef struct {
    int needed;
    SDL_AudioFormat src_format, dst_format;
    double rate_incr;
    Uint8* buf; int len, len_cvt, len_mult; double len_ratio;
    void (*filters[10])(void);
    int filter_index;
} SDL_AudioCVT;
int  SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained);
void SDL_CloseAudio(void);
void SDL_PauseAudio(int on);
void SDL_LockAudio(void);
void SDL_UnlockAudio(void);
int  SDL_GetAudioStatus(void);
int  SDL_BuildAudioCVT(SDL_AudioCVT* cvt, SDL_AudioFormat src_fmt, Uint8 src_ch, int src_rate,
                       SDL_AudioFormat dst_fmt, Uint8 dst_ch, int dst_rate);
int  SDL_ConvertAudio(SDL_AudioCVT* cvt);

/* Joystick / haptic — minimal stubs */
int  SDL_NumJoysticks(void);
SDL_GameController* SDL_GameControllerOpen(int index);
void SDL_GameControllerClose(SDL_GameController*);
SDL_bool SDL_IsGameController(int index);
SDL_GameController* SDL_GameControllerFromInstanceID(int);
SDL_Joystick* SDL_JoystickOpen(int);
int SDL_GameControllerAddMappingsFromFile(const char* file);
SDL_Haptic* SDL_HapticOpen(int);
int  SDL_HapticRumbleInit(SDL_Haptic*);
int  SDL_HapticRumblePlay(SDL_Haptic*, float, Uint32);

enum {
    SDL_CONTROLLER_AXIS_LEFTX = 0,
    SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_AXIS_MAX
};
enum {
    SDL_CONTROLLER_BUTTON_A=0, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y, SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK, SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_BUTTON_MAX
};

#define SDL_BUTTON_LEFT   1
#define SDL_BUTTON_RIGHT  3
#define SDL_BUTTON_X1     4

/* Window events */
enum {
    SDL_WINDOWEVENT_NONE = 0,
    SDL_WINDOWEVENT_SHOWN,
    SDL_WINDOWEVENT_HIDDEN,
    SDL_WINDOWEVENT_EXPOSED,
    SDL_WINDOWEVENT_MOVED,
    SDL_WINDOWEVENT_RESIZED,
    SDL_WINDOWEVENT_SIZE_CHANGED,
    SDL_WINDOWEVENT_MINIMIZED,
    SDL_WINDOWEVENT_MAXIMIZED,
    SDL_WINDOWEVENT_RESTORED,
    SDL_WINDOWEVENT_ENTER,
    SDL_WINDOWEVENT_LEAVE,
    SDL_WINDOWEVENT_FOCUS_GAINED,
    SDL_WINDOWEVENT_FOCUS_LOST,
    SDL_WINDOWEVENT_CLOSE
};
#define SDL_APPACTIVE       0
#define SDL_APPINPUTFOCUS   0

/* Timers (1 shot via tag — SDLPoP only uses one global timer) */
typedef Uint32 (*SDL_TimerCallback)(Uint32 interval, void* param);
SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_TimerCallback cb, void* param);
int         SDL_RemoveTimer(SDL_TimerID id);
void        SDL_PumpEvents(void);

/* iconv / strings */
char* SDL_iconv_string(const char* tocode, const char* fromcode, const char* inbuf, size_t inbytesleft);
void  SDL_free(void* mem);
size_t SDL_wcslen(const Uint16* w);
size_t SDL_strlen(const char* s);

/* Compile-time assertion. */
#define SDL_COMPILE_TIME_ASSERT(name, x) typedef int SDL_dummy_##name[(x) * 2 - 1]

/* Version helper */
typedef struct { Uint8 major, minor, patch; } SDL_version;
#define SDL_VERSION(v) do { (v)->major=2; (v)->minor=0; (v)->patch=5; } while(0)
#define SDL_GetVersion SDL_VERSION
#define SDL_VERSION_ATLEAST(a,b,c) ((2*10000+0*100+5) >= ((a)*10000+(b)*100+(c)))

#define SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING "SDL_WINDOWS_DISABLE_THREAD_NAMING"

/* Removed in SDL2 but referenced by SDLPoP under #ifdef paths we never enable. */
#define SDL_EnableKeyRepeat(a,b) 0
#define SDL_EnableUNICODE(x) 0
#define SDL_WM_SetCaption(a,b) ((void)0)

#ifdef __cplusplus
}
#endif

#endif /* SDL_GBA_SHIM_H */
