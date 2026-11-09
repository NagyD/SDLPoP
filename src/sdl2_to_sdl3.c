/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2024  Dávid Nagy

SDL2 -> SDL3 audio bridge (see sdl2_to_sdl3.h).

SDL3 removed the callback-based push-audio API. The game still fills audio in a
SDL2-style callback (audio_callback in seg009.c), so we open an SDL_AudioStream
with SDL_OpenAudioDeviceStream and, whenever the device needs more data, run the
game's callback into a scratch buffer and push it to the stream.

The stream handle and the desired spec are single shared globals: the audio
callback runs on SDL's audio thread while seg009.c and midi.c call
SDL_LockAudio/SDL_PauseAudio around state changes, so all translation units must
act on the same stream. Hence this lives in a .c file, not the header.
*/

#include "common.h" /* pulls in config.h + types.h + SDL3 + sdl2_to_sdl3.h (SDL_AudioSpec == compat) */

#ifdef USE_SDL3

/* Restore the real SDL3 SDL_AudioSpec for opening the device; the SDL2-facing
   spec keeps its explicit typedef name compat_SDL_AudioSpec. */
#undef SDL_AudioSpec

static SDL_AudioStream*     s_audio_stream = NULL;
static compat_SDL_AudioSpec s_audio_desired;

static void SDLCALL compat_audio_bridge(void* userdata, SDL_AudioStream* stream,
                                        int additional_amount, int total_amount) {
	(void)userdata; (void)total_amount;
	if (additional_amount <= 0) return;
	Uint8* buf = (Uint8*)SDL_malloc((size_t)additional_amount);
	if (buf == NULL) return;
	if (s_audio_desired.callback != NULL) {
		s_audio_desired.callback(s_audio_desired.userdata, buf, additional_amount);
	} else {
		SDL_memset(buf, s_audio_desired.silence, (size_t)additional_amount);
	}
	SDL_PutAudioStreamData(stream, buf, additional_amount);
	SDL_free(buf);
}

int compat_SDL_OpenAudio(compat_SDL_AudioSpec* desired, compat_SDL_AudioSpec* obtained) {
	if (desired == NULL) return -1;
	/* SDL2's SDL_OpenAudio implicitly initialized the audio subsystem; SDL3's
	   stream API does not, and the game only requests video+gamepad in SDL_Init. */
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	s_audio_desired = *desired;
	/* Silence value the game memsets with (0x80 for unsigned 8-bit, else 0). */
	s_audio_desired.silence = SDL_AUDIO_ISUNSIGNED(desired->format) ? 0x80 : 0x00;
	desired->silence = s_audio_desired.silence;

	SDL_AudioSpec spec;
	SDL_zero(spec);
	spec.freq = desired->freq;
	spec.format = desired->format;
	spec.channels = desired->channels;

	s_audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
	                                           &spec, compat_audio_bridge, NULL);
	if (s_audio_stream == NULL) return -1;
	/* Streams from SDL_OpenAudioDeviceStream start paused, like SDL2's device
	   after SDL_OpenAudio; the game calls SDL_PauseAudio(0) to start playback. */
	if (obtained != NULL) *obtained = s_audio_desired;
	return 0;
}

void compat_SDL_PauseAudio(int pause_on) {
	if (s_audio_stream == NULL) return;
	if (pause_on) SDL_PauseAudioStreamDevice(s_audio_stream);
	else          SDL_ResumeAudioStreamDevice(s_audio_stream);
}

void compat_SDL_LockAudio(void)   { if (s_audio_stream) SDL_LockAudioStream(s_audio_stream); }
void compat_SDL_UnlockAudio(void) { if (s_audio_stream) SDL_UnlockAudioStream(s_audio_stream); }

void compat_SDL_CloseAudio(void) {
	if (s_audio_stream) { SDL_DestroyAudioStream(s_audio_stream); s_audio_stream = NULL; }
}

void compat_SDL_AudioResampleFF(SDL_AudioFormat fmt, int channels, int src_freq, int dst_freq,
                                const Uint8* in, int in_len, Uint8* out, int out_len) {
	SDL_AudioSpec src, dst;
	SDL_zero(src); SDL_zero(dst);
	src.format = fmt; src.channels = channels; src.freq = src_freq;
	dst.format = fmt; dst.channels = channels; dst.freq = dst_freq;
	Uint8* converted = NULL;
	int converted_len = 0;
	if (SDL_ConvertAudioSamples(&src, in, in_len, &dst, &converted, &converted_len)) {
		int n = (out_len < converted_len) ? out_len : converted_len;
		SDL_memcpy(out, converted, (size_t)n);
		SDL_free(converted);
	} else {
		int n = (out_len < in_len) ? out_len : in_len; /* fallback: no resample */
		SDL_memcpy(out, in, (size_t)n);
	}
}

#endif /* USE_SDL3 */
