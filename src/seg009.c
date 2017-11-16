/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2017  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

The authors of this program may be contacted at http://forum.princed.org
*/

#include "common.h"
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

// Most functions in this file are different from those in the original game.

void sdlperror(const char* header) {
	const char* error = SDL_GetError();
	printf("%s: %s\n",header,error);
	//quit(1);
}

dat_type* dat_chain_ptr = NULL;

int last_key_scancode;
char last_text_input;

// seg009:000D
int __pascal far read_key() {
	// stub
	int key = last_key_scancode;
	last_key_scancode = 0;
	return key;
}

// seg009:019A
void __pascal far clear_kbd_buf() {
	// stub
	last_key_scancode = 0;
	last_text_input = 0;
}

// seg009:040A
word __pascal far prandom(word max) {
	if (!seed_was_init) {
		// init from current time
		random_seed = time(NULL);
		seed_was_init = 1;
	}
	random_seed = random_seed * 214013 + 2531011;
	return (random_seed >> 16) % (max + 1);
}

// seg009:0467
int __pascal far round_xpos_to_byte(int xpos,int round_direction) {
	// stub
	return xpos;
}

// seg009:0C7A
void __pascal far quit(int exit_code) {
	restore_stuff();
	exit(exit_code);
}

// seg009:0C90
void __pascal far restore_stuff() {
	SDL_Quit();
}

// seg009:0E33
int __pascal far key_test_quit() {
	word key;
	key = read_key();
	if (key == (SDL_SCANCODE_Q | WITH_CTRL)) { // ctrl-q

		#ifdef USE_REPLAY
		if (recording) save_recorded_replay();
		#endif

		quit(0);
	}
	return key;
}

// seg009:0E54
const char* __pascal far check_param(const char *param) {
	// stub
	short arg_index;
	for (arg_index = 1; arg_index < g_argc; ++arg_index) {

		char* curr_arg = g_argv[arg_index];

		// Filenames (e.g. replays) should never be a valid 'normal' param so we should skip these to prevent conflicts.
		// We can lazily distinguish filenames from non-filenames by checking whether they have a dot in them.
		// (Assumption: all relevant files, e.g. replay files, have some file extension anyway)
		if (strchr(curr_arg, '.') != NULL) {
			continue;
		}

		// List of params that expect a specifier ('sub-') arg directly after it (e.g. the mod's name, after "mod" arg)
		// Such sub-args may conflict with the normal params (so, we should 'skip over' them)
		static const char params_with_one_subparam[][16] = { "mod", "validate", /*...*/ };

		bool curr_arg_has_one_subparam = false;
		int i;
		for (i = 0; i < COUNT(params_with_one_subparam); ++i) {
			if (strncasecmp(curr_arg, params_with_one_subparam[i], strlen(params_with_one_subparam[i])) == 0) {
				curr_arg_has_one_subparam = true;
				break;
			}
		}

		if (curr_arg_has_one_subparam) {
			// Found an arg that has one sub-param, so we want to:
			// 1: skip over the next arg                (if we are NOT checking for this specific param)
			// 2: return a pointer below to the SUB-arg (if we ARE checking for this specific param)
			++arg_index;
			if (!(arg_index < g_argc)) return NULL; // not enough arguments
		}

		if (/*strnicmp*/strncasecmp(curr_arg, param, strlen(param)) == 0) {
			return g_argv[arg_index];
		}
	}
	return NULL;
}

// seg009:0EDF
int __pascal far pop_wait(int timer_index,int time) {
	start_timer(timer_index, time);
	return do_wait(timer_index);
}

// S_ISREG may not be defined under MSVC
#if defined(_MSC_VER) && !defined(S_ISREG)
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

static FILE* open_dat_from_root_or_data_dir(const char* filename) {
	FILE* fp = NULL;
	fp = fopen(filename, "rb");

	// if failed, try if the DAT file can be opened in the data/ directory, instead of the main folder
	if (fp == NULL) {
		char data_path[POP_MAX_PATH];
		snprintf(data_path, sizeof(data_path), "data/%s", filename);

		// verify that this is a regular file and not a directory (otherwise, don't open)
		struct stat path_stat;
		stat(data_path, &path_stat);
		if (S_ISREG(path_stat.st_mode)) {
			fp = fopen(data_path, "rb");
		}
	}
	return fp;
}

// seg009:0F58
dat_type *__pascal open_dat(const char *filename,int drive) {
	FILE* fp = NULL;
	if (!use_custom_levelset) {
		fp = open_dat_from_root_or_data_dir(filename);
	}
	else {
		char filename_mod[POP_MAX_PATH];
		// before checking the root directory, first try mods/MODNAME/
		snprintf(filename_mod, sizeof(filename_mod), "mods/%s/%s", levelset_name, filename);
		fp = fopen(filename_mod, "rb");
		if (fp == NULL) {
			fp = open_dat_from_root_or_data_dir(filename);
		}
	}
	dat_header_type dat_header;
	dat_table_type* dat_table = NULL;

	dat_type* pointer = (dat_type*) calloc(1, sizeof(dat_type));
	strncpy(pointer->filename, filename, sizeof(pointer->filename));
	pointer->next_dat = dat_chain_ptr;
	dat_chain_ptr = pointer;

	if (fp != NULL) {
		if (fread(&dat_header, 6, 1, fp) != 1)
			goto failed;
		dat_table = (dat_table_type*) malloc(dat_header.table_size);
		if (dat_table == NULL ||
		    fseek(fp, dat_header.table_offset, SEEK_SET) ||
		    fread(dat_table, dat_header.table_size, 1, fp) != 1)
			goto failed;
		pointer->handle = fp;
		pointer->dat_table = dat_table;
	}
out:
	// stub
	return pointer;
failed:
	perror(filename);
	if (fp)
		fclose(fp);
	if (dat_table)
		free(dat_table);
	goto out;
}

// seg009:9CAC
void __pascal far set_loaded_palette(dat_pal_type far *palette_ptr) {
	int dest_row, dest_index, source_row;
	for (dest_row = dest_index = source_row = 0; dest_row < 16; ++dest_row, dest_index += 0x10) {
		if (palette_ptr->row_bits & (1 << dest_row)) {
			set_pal_arr(dest_index, 16, palette_ptr->vga + source_row*0x10, 1);
			++source_row;
		}
	}
}

// data:3356
word chtab_palette_bits = 1;

// seg009:104E
chtab_type* __pascal load_sprites_from_file(int resource,int palette_bits, int quit_on_error) {
	int i;
	int n_images = 0;
	//int has_palette_bits = 1;
	chtab_type* chtab = NULL;
	dat_shpl_type* shpl = (dat_shpl_type*) load_from_opendats_alloc(resource, "pal", NULL, NULL);
	if (shpl == NULL) {
		printf("Can't load sprites from resource %d.\n", resource);
		//if (quit_on_error) quit(1);
		return NULL;
	}

	dat_pal_type* pal_ptr = &shpl->palette;
	if (graphics_mode == gmMcgaVga) {
		if (palette_bits == 0) {
			/*
			palette_bits = add_palette_bits(pal_ptr->n_colors);
			if (palette_bits == 0) {
				quit(1);
			}
			*/
		} else {
			chtab_palette_bits |= palette_bits;
			//has_palette_bits = 0;
		}
		pal_ptr->row_bits = palette_bits;
	}

	n_images = shpl->n_images;
	size_t alloc_size = sizeof(chtab_type) + sizeof(void far *) * n_images;
	chtab = (chtab_type*) malloc(alloc_size);
	memset(chtab, 0, alloc_size);
	chtab->n_images = n_images;
	for (i = 1; i <= n_images; i++) {
		SDL_Surface* image = load_image(resource + i, pal_ptr);
//		if (image == NULL) printf(" failed");
		if (image != NULL) {

			if (SDL_SetSurfaceAlphaMod(image, 0) != 0) {
				sdlperror("SDL_SetAlpha");
				quit(1);
			}

			/*
			if (SDL_SetColorKey(image, SDL_SRCCOLORKEY, 0) != 0) {
				sdlperror("SDL_SetColorKey");
				quit(1);
			}
			*/
		}
//		printf("\n");
		chtab->images[i-1] = image;
	}
	set_loaded_palette(pal_ptr);
	return chtab;
}

// seg009:11A8
void __pascal far free_chtab(chtab_type *chtab_ptr) {
	image_type far* curr_image;
	word id;
	word n_images;
	if (graphics_mode == gmMcgaVga && chtab_ptr->has_palette_bits) {
		chtab_palette_bits &= ~ chtab_ptr->chtab_palette_bits;
	}
	n_images = chtab_ptr->n_images;
	for (id = 0; id < n_images; ++id) {
		curr_image = chtab_ptr->images[id];
		if (curr_image) {
			SDL_FreeSurface(curr_image);
		}
	}
	free_near(chtab_ptr);
}

// seg009:8CE6
void __pascal far decompress_rle_lr(byte far *destination,const byte far *source,int dest_length) {
	const byte* src_pos = source;
	byte* dest_pos = destination;
	short rem_length = dest_length;
	while (rem_length) {
		sbyte count = *(src_pos++);
		if (count >= 0) { // copy
			++count;
			do {
				*(dest_pos++) = *(src_pos++);
				--rem_length;
			} while (--count && rem_length);
		} else { // repeat
			byte al = *(src_pos++);
			count = -count;
			do {
				*(dest_pos++) = al;
				--rem_length;
			} while (--count && rem_length);
		}
	}
}

// seg009:8D1C
void __pascal far decompress_rle_ud(byte far *destination,const byte far *source,int dest_length,int width,int height) {
	short rem_height = height;
	const byte* src_pos = source;
	byte* dest_pos = destination;
	short rem_length = dest_length;
	--dest_length;
	--width;
	while (rem_length) {
		sbyte count = *(src_pos++);
		if (count >= 0) { // copy
			++count;
			do {
				*(dest_pos++) = *(src_pos++);
				dest_pos += width;
				if (--rem_height == 0) {
					dest_pos -= dest_length;
					rem_height = height;
				}
				--rem_length;
			} while (--count && rem_length);
		} else { // repeat
			byte al = *(src_pos++);
			count = -count;
			do {
				*(dest_pos++) = al;
				dest_pos += width;
				if (--rem_height == 0) {
					dest_pos -= dest_length;
					rem_height = height;
				}
				--rem_length;
			} while (--count && rem_length);
		}
	}
}

// seg009:90FA
byte far* __pascal far decompress_lzg_lr(byte far *dest,const byte far *source,int dest_length) {
	byte* window = (byte*) malloc_near(0x400);
	if (window == NULL) return NULL;
	memset(window, 0, 0x400);
	byte* window_pos = window + 0x400 - 0x42; // bx
	short remaining = dest_length; // cx
	byte* window_end = window + 0x400; // dx
	const byte* source_pos = source;
	byte* dest_pos = dest;
	word mask = 0;
	do {
		mask >>= 1;
		if ((mask & 0xFF00) == 0) {
			mask = *(source_pos++) | 0xFF00;
		}
		if (mask & 1) {
			*(window_pos++) = *(dest_pos++) = *(source_pos++);
			if (window_pos >= window_end) window_pos = window;
			--remaining;
		} else {
			word copy_info = *(source_pos++);
			copy_info = (copy_info << 8) | *(source_pos++);
			byte* copy_source = window + (copy_info & 0x3FF);
			byte copy_length = (copy_info >> 10) + 3;
			do {
				*(window_pos++) = *(dest_pos++) = *(copy_source++);
				if (copy_source >= window_end) copy_source = window;
				if (window_pos >= window_end) window_pos = window;
			} while (--remaining && --copy_length);
		}
	} while (remaining);
//	end:
	free(window);
	return dest;
}

// seg009:91AD
byte far* __pascal far decompress_lzg_ud(byte far *dest,const byte far *source,int dest_length,int stride,int height) {
	byte* window = (byte*) malloc_near(0x400);
	if (window == NULL) return NULL;
	memset(window, 0, 0x400);
	byte* window_pos = window + 0x400 - 0x42; // bx
	short remaining = height; // cx
	byte* window_end = window + 0x400; // dx
	const byte* source_pos = source;
	byte* dest_pos = dest;
	word mask = 0;
	short var_6 = dest_length - 1;
	do {
		mask >>= 1;
		if ((mask & 0xFF00) == 0) {
			mask = *(source_pos++) | 0xFF00;
		}
		if (mask & 1) {
			*(window_pos++) = *dest_pos = *(source_pos++);
			dest_pos += stride;
			if (--remaining == 0) {
				dest_pos -= var_6;
				remaining = height;
			}
			if (window_pos >= window_end) window_pos = window;
			--dest_length;
		} else {
			word copy_info = *(source_pos++);
			copy_info = (copy_info << 8) | *(source_pos++);
			byte* copy_source = window + (copy_info & 0x3FF);
			byte copy_length = (copy_info >> 10) + 3;
			do {
				*(window_pos++) = *dest_pos = *(copy_source++);
				dest_pos += stride;
				if (--remaining == 0) {
					dest_pos -= var_6;
					remaining = height;
				}
				if (copy_source >= window_end) copy_source = window;
				if (window_pos >= window_end) window_pos = window;
			} while (--dest_length && --copy_length);
		}
	} while (dest_length);
//	end:
	free(window);
	return dest;
}

// seg009:938E
void __pascal far decompr_img(byte far *dest,const image_data_type far *source,int decomp_size,int cmeth, int stride) {
	switch (cmeth) {
		case 0: // RAW left-to-right
			memcpy_far(dest, &source->data, decomp_size);
		break;
		case 1: // RLE left-to-right
			decompress_rle_lr(dest, source->data, decomp_size);
		break;
		case 2: // RLE up-to-down
			decompress_rle_ud(dest, source->data, decomp_size, stride, source->height);
		break;
		case 3: // LZG left-to-right
			decompress_lzg_lr(dest, source->data, decomp_size);
		break;
		case 4: // LZG up-to-down
			decompress_lzg_ud(dest, source->data, decomp_size, stride, source->height);
		break;
	}
}

int calc_stride(image_data_type* image_data) {
	int width = image_data->width;
	int flags = image_data->flags;
	int depth = ((flags >> 12) & 3) + 1;
	return (depth * width + 7) / 8;
}

byte* conv_to_8bpp(byte* in_data, int width, int height, int stride, int depth) {
	byte* out_data = (byte*) malloc(width * height);
	int y, x_pixel, x_byte, pixel_in_byte;
	int pixels_per_byte = 8 / depth;
	int mask = (1 << depth) - 1;
	for (y = 0; y < height; ++y) {
		byte* in_pos = in_data + y*stride;
		byte* out_pos = out_data + y*width;
		for (x_pixel = x_byte = 0; x_byte < stride; ++x_byte) {
			byte v = *in_pos;
			int shift = 8;
			for (pixel_in_byte = 0; pixel_in_byte < pixels_per_byte && x_pixel < width; ++pixel_in_byte, ++x_pixel) {
				shift -= depth;
				*out_pos = (v >> shift) & mask;
				++out_pos;
			}
			++in_pos;
		}
	}
	return out_data;
}

image_type* decode_image(image_data_type* image_data, dat_pal_type* palette) {
	int height = image_data->height;
	if (height == 0) return NULL;
	int width = image_data->width;
	int flags = image_data->flags;
	int depth = ((flags >> 12) & 3) + 1;
	int cmeth = (flags >> 8) & 0x0F;
	int stride = calc_stride(image_data);
	int dest_size = stride * height;
	byte* dest = (byte*) malloc(dest_size);
	memset(dest, 0, dest_size);
	decompr_img(dest, image_data, dest_size, cmeth, stride);
	byte* image_8bpp = conv_to_8bpp(dest, width, height, stride, depth);
	free(dest); dest = NULL;
	image_type* image = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
	if (image == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	if (SDL_LockSurface(image) != 0) {
		sdlperror("SDL_LockSurface");
	}
	int y;
	for (y = 0; y < height; ++y) {
		// fill image with data
		memcpy((byte*)image->pixels + y*image->pitch, image_8bpp + y*width, width);
	}
	SDL_UnlockSurface(image);

	free(image_8bpp); image_8bpp = NULL;
	SDL_Color colors[16];
	int i;
	for (i = 0; i < 16; ++i) {
		colors[i].r = palette->vga[i].r << 2;
		colors[i].g = palette->vga[i].g << 2;
		colors[i].b = palette->vga[i].b << 2;
		colors[i].a = SDL_ALPHA_OPAQUE;   // SDL2's SDL_Color has a fourth alpha component
	}
	colors[0].a = SDL_ALPHA_TRANSPARENT;
	SDL_SetPaletteColors(image->format->palette, colors, 0, 16); // SDL_SetColors = deprecated
	return image;
}

// seg009:121A
image_type* far __pascal far load_image(int resource_id, dat_pal_type* palette) {
	// stub
	data_location result;
	int size;
	void* image_data = load_from_opendats_alloc(resource_id, "png", &result, &size);
	image_type* image = NULL;
	switch (result) {
		case data_none:
			return NULL;
		break;
		case data_DAT: { // DAT
			image = decode_image((image_data_type*) image_data, palette);
		} break;
		case data_directory: { // directory
			SDL_RWops* rw = SDL_RWFromConstMem(image_data, size);
			if (rw == NULL) {
				sdlperror("SDL_RWFromConstMem");
				return NULL;
			}
			image = IMG_LoadPNG_RW(rw);
			if (SDL_RWclose(rw) != 0) {
				sdlperror("SDL_RWclose");
			}
		} break;
	}
	if (image_data != NULL) free(image_data);


	if (image != NULL) {
		// should immediately start using the onscreen pixel format, so conversion will not be needed

		if (SDL_SetColorKey(image, SDL_TRUE, 0) != 0) { //sdl 1.2: SDL_SRCCOLORKEY
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
//		printf("bpp = %d\n", image->format->BitsPerPixel);
		if (SDL_SetSurfaceAlphaMod(image, 0) != 0) { //sdl 1.2: SDL_SetAlpha removed
			sdlperror("SDL_SetAlpha");
			quit(1);
		}
//		image_type* colored_image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ARGB8888, 0);
//		if (!colored_image) {
//			sdlperror("SDL_ConvertSurfaceFormat");
//			quit(1);
//		}
//		SDL_FreeSurface(image);
//		image = colored_image;
	}
	return image;
}

// seg009:13C4
void __pascal far draw_image_transp(image_type far *image,image_type far *mask,int xpos,int ypos) {
	if (graphics_mode == gmMcgaVga) {
		draw_image_transp_vga(image, xpos, ypos);
	} else {
		// ...
	}
}

// seg009:157E
int __pascal far set_joy_mode() {
	// stub
	if (SDL_NumJoysticks() < 1) {
		is_joyst_mode = 0;
	} else {
		if (SDL_IsGameController(0)) {
			sdl_controller_ = SDL_GameControllerOpen(0);
			if (sdl_controller_ == NULL) {
				is_joyst_mode = 0;
			} else {
				is_joyst_mode = 1;
			}
		}
		// We have a joystick connected, but it's NOT compatible with the SDL_GameController
		// interface, so we resort to the classic SDL_Joystick interface instead
		else {
			sdl_joystick_ = SDL_JoystickOpen(0);
			is_joyst_mode = 1;
		}
	}
	if (enable_controller_rumble && is_joyst_mode) {
		sdl_haptic = SDL_HapticOpen(0);
		SDL_HapticRumbleInit(sdl_haptic); // initialize the device for simple rumble
	} else {
		sdl_haptic = NULL;
	}

	is_keyboard_mode = !is_joyst_mode;
	return is_joyst_mode;
}

// seg009:178B
surface_type far *__pascal make_offscreen_buffer(const rect_type far *rect) {
	// stub
#ifndef USE_ALPHA
	// Bit order matches onscreen buffer, good for fading.
	return SDL_CreateRGBSurface(0, rect->right, rect->bottom, 24, 0xFF, 0xFF<<8, 0xFF<<16, 0); //RGB888 (little endian)
#else
	return SDL_CreateRGBSurface(0, rect->right, rect->bottom, 32, 0xFF, 0xFF<<8, 0xFF<<16, 0xFF<<24);
#endif
	//return surface;
}

// seg009:17BD
void __pascal far free_surface(surface_type *surface) {
	SDL_FreeSurface(surface);
}

// seg009:17EA
void __pascal far free_peel(peel_type *peel_ptr) {
	SDL_FreeSurface(peel_ptr->peel);
	free(peel_ptr);
}

// seg009:182F
void __pascal far set_hc_pal() {
	// stub
	if (graphics_mode == gmMcgaVga) {
		set_pal_arr(0, 16, vga_palette, 1);
	} else {
		// ...
	}
}

// seg009:2446
void __pascal far flip_not_ega(byte far *memory,int height,int stride) {
	byte* row_buffer = (byte*) malloc(stride);
	byte* top_ptr;
	byte* bottom_ptr;
	bottom_ptr = top_ptr = memory;
	bottom_ptr += (height - 1) * stride;
	short rem_rows = height >> 1;
	do {
		memcpy(row_buffer, top_ptr, stride);
		memcpy(top_ptr, bottom_ptr, stride);
		memcpy(bottom_ptr, row_buffer, stride);
		top_ptr += stride;
		bottom_ptr -= stride;
		--rem_rows;
	} while (rem_rows);
	free(row_buffer);
}

// seg009:19B1
void __pascal far flip_screen(surface_type far *surface) {
	// stub
	if (graphics_mode != gmEga) {
		if (SDL_LockSurface(surface) != 0) {
			sdlperror("SDL_LockSurface");
			quit(1);
		}
		flip_not_ega((byte*) surface->pixels, surface->h, surface->pitch);
		SDL_UnlockSurface(surface);
	} else {
		// ...
	}
}

#ifndef USE_FADE
// seg009:19EF
void __pascal far fade_in_2(surface_type near *source_surface,int which_rows) {
	// stub
	method_1_blit_rect(onscreen_surface_, source_surface, &screen_rect, &screen_rect, 0);
}

// seg009:1CC9
void __pascal far fade_out_2(int rows) {
	// stub
}
#endif // USE_FADE

// seg009:2288
void __pascal far draw_image_transp_vga(image_type far *image,int xpos,int ypos) {
	// stub
	method_6_blit_img_to_scr(image, xpos, ypos, blitters_10h_transp);
}

#ifdef USE_TEXT

font_type hc_font = {0x01,0xFF, 7,2,1,1, NULL};
textstate_type textstate = {0,0,0,15,&hc_font};

/*const*/ byte hc_font_data[] = {
0x20,0x83,0x07,0x00,0x02,0x00,0x01,0x00,0x01,0x00,0xD2,0x00,0xD8,0x00,0xE5,0x00,
0xEE,0x00,0xFA,0x00,0x07,0x01,0x14,0x01,0x21,0x01,0x2A,0x01,0x37,0x01,0x44,0x01,
0x50,0x01,0x5C,0x01,0x6A,0x01,0x74,0x01,0x81,0x01,0x8E,0x01,0x9B,0x01,0xA8,0x01,
0xB5,0x01,0xC2,0x01,0xCF,0x01,0xDC,0x01,0xE9,0x01,0xF6,0x01,0x03,0x02,0x10,0x02,
0x1C,0x02,0x2A,0x02,0x37,0x02,0x42,0x02,0x4F,0x02,0x5C,0x02,0x69,0x02,0x76,0x02,
0x83,0x02,0x90,0x02,0x9D,0x02,0xAA,0x02,0xB7,0x02,0xC4,0x02,0xD1,0x02,0xDE,0x02,
0xEB,0x02,0xF8,0x02,0x05,0x03,0x12,0x03,0x1F,0x03,0x2C,0x03,0x39,0x03,0x46,0x03,
0x53,0x03,0x60,0x03,0x6D,0x03,0x7A,0x03,0x87,0x03,0x94,0x03,0xA1,0x03,0xAE,0x03,
0xBB,0x03,0xC8,0x03,0xD5,0x03,0xE2,0x03,0xEB,0x03,0xF9,0x03,0x02,0x04,0x0F,0x04,
0x1C,0x04,0x29,0x04,0x36,0x04,0x43,0x04,0x50,0x04,0x5F,0x04,0x6C,0x04,0x79,0x04,
0x88,0x04,0x95,0x04,0xA2,0x04,0xAF,0x04,0xBC,0x04,0xC9,0x04,0xD8,0x04,0xE7,0x04,
0xF4,0x04,0x01,0x05,0x0E,0x05,0x1B,0x05,0x28,0x05,0x35,0x05,0x42,0x05,0x51,0x05,
0x5E,0x05,0x6B,0x05,0x78,0x05,0x85,0x05,0x8D,0x05,0x9A,0x05,0xA7,0x05,0xBB,0x05,
0xD9,0x05,0x00,0x00,0x03,0x00,0x00,0x00,0x07,0x00,0x02,0x00,0x01,0x00,0xC0,0xC0,
0xC0,0xC0,0xC0,0x00,0xC0,0x03,0x00,0x05,0x00,0x01,0x00,0xD8,0xD8,0xD8,0x06,0x00,
0x07,0x00,0x01,0x00,0x00,0x6C,0xFE,0x6C,0xFE,0x6C,0x07,0x00,0x07,0x00,0x01,0x00,
0x10,0x7C,0xD0,0x7C,0x16,0x7C,0x10,0x07,0x00,0x08,0x00,0x01,0x00,0xC3,0xC6,0x0C,
0x18,0x30,0x63,0xC3,0x07,0x00,0x08,0x00,0x01,0x00,0x38,0x6C,0x38,0x7A,0xCC,0xCE,
0x7B,0x03,0x00,0x03,0x00,0x01,0x00,0x60,0x60,0xC0,0x07,0x00,0x04,0x00,0x01,0x00,
0x30,0x60,0xC0,0xC0,0xC0,0x60,0x30,0x07,0x00,0x04,0x00,0x01,0x00,0xC0,0x60,0x30,
0x30,0x30,0x60,0xC0,0x06,0x00,0x07,0x00,0x01,0x00,0x00,0x6C,0x38,0xFE,0x38,0x6C,
0x06,0x00,0x06,0x00,0x01,0x00,0x00,0x30,0x30,0xFC,0x30,0x30,0x08,0x00,0x03,0x00,
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0xC0,0x04,0x00,0x04,0x00,0x01,0x00,
0x00,0x00,0x00,0xF0,0x07,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,
0xC0,0x07,0x00,0x08,0x00,0x01,0x00,0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0x07,0x00,
0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,
0x00,0x30,0x70,0xF0,0x30,0x30,0x30,0xFC,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,
0x0C,0x18,0x30,0x60,0xFC,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0x0C,0x18,0x0C,
0xCC,0x78,0x07,0x00,0x07,0x00,0x01,0x00,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x07,
0x00,0x06,0x00,0x01,0x00,0xF8,0xC0,0xC0,0xF8,0x0C,0x0C,0xF8,0x07,0x00,0x06,0x00,
0x01,0x00,0x78,0xC0,0xC0,0xF8,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0xFC,
0x0C,0x18,0x30,0x30,0x30,0x30,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0x78,
0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,
0x06,0x00,0x02,0x00,0x01,0x00,0x00,0xC0,0xC0,0x00,0xC0,0xC0,0x08,0x00,0x03,0x00,
0x01,0x00,0x00,0x60,0x60,0x00,0x00,0x60,0x60,0xC0,0x07,0x00,0x05,0x00,0x01,0x00,
0x18,0x30,0x60,0xC0,0x60,0x30,0x18,0x05,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0xF0,
0x00,0xF0,0x07,0x00,0x05,0x00,0x01,0x00,0xC0,0x60,0x30,0x18,0x30,0x60,0xC0,0x07,
0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0x0C,0x18,0x30,0x00,0x30,0x07,0x00,0x06,0x00,
0x01,0x00,0x78,0xCC,0xDC,0xDC,0xD8,0xC0,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x78,
0xCC,0xCC,0xFC,0xCC,0xCC,0xCC,0x07,0x00,0x06,0x00,0x01,0x00,0xF8,0xCC,0xCC,0xF8,
0xCC,0xCC,0xF8,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xC0,0xC0,0xC0,0xCC,0x78,
0x07,0x00,0x06,0x00,0x01,0x00,0xF8,0xCC,0xCC,0xCC,0xCC,0xCC,0xF8,0x07,0x00,0x05,
0x00,0x01,0x00,0xF8,0xC0,0xC0,0xF0,0xC0,0xC0,0xF8,0x07,0x00,0x05,0x00,0x01,0x00,
0xF8,0xC0,0xC0,0xF0,0xC0,0xC0,0xC0,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xC0,
0xDC,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0xCC,0xFC,0xCC,0xCC,
0xCC,0x07,0x00,0x04,0x00,0x01,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0xF0,0x07,0x00,
0x06,0x00,0x01,0x00,0x0C,0x0C,0x0C,0x0C,0x0C,0xCC,0x78,0x07,0x00,0x07,0x00,0x01,
0x00,0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x07,0x00,0x05,0x00,0x01,0x00,0xC0,0xC0,
0xC0,0xC0,0xC0,0xC0,0xF8,0x07,0x00,0x08,0x00,0x01,0x00,0xC3,0xE7,0xFF,0xDB,0xC3,
0xC3,0xC3,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0xEC,0xFC,0xDC,0xCC,0xCC,0x07,
0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,
0x01,0x00,0xF8,0xCC,0xCC,0xF8,0xC0,0xC0,0xC0,0x07,0x00,0x06,0x00,0x01,0x00,0x78,
0xCC,0xCC,0xCC,0xCC,0xD8,0x6C,0x07,0x00,0x06,0x00,0x01,0x00,0xF8,0xCC,0xCC,0xF8,
0xD8,0xCC,0xCC,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xC0,0x78,0x0C,0xCC,0x78,
0x07,0x00,0x06,0x00,0x01,0x00,0xFC,0x30,0x30,0x30,0x30,0x30,0x30,0x07,0x00,0x06,
0x00,0x01,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x07,0x00,0x06,0x00,0x01,0x00,
0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x30,0x07,0x00,0x08,0x00,0x01,0x00,0xC3,0xC3,0xC3,
0xDB,0xFF,0xE7,0xC3,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0x78,0x30,0x78,0xCC,
0xCC,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0xCC,0x78,0x30,0x30,0x30,0x07,0x00,
0x08,0x00,0x01,0x00,0xFF,0x06,0x0C,0x18,0x30,0x60,0xFF,0x07,0x00,0x04,0x00,0x01,
0x00,0xF0,0xC0,0xC0,0xC0,0xC0,0xC0,0xF0,0x07,0x00,0x08,0x00,0x01,0x00,0xC0,0x60,
0x30,0x18,0x0C,0x06,0x03,0x07,0x00,0x04,0x00,0x01,0x00,0xF0,0x30,0x30,0x30,0x30,
0x30,0xF0,0x03,0x00,0x06,0x00,0x01,0x00,0x30,0x78,0xCC,0x08,0x00,0x06,0x00,0x01,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x03,0x00,0x04,0x00,0x01,0x00,0xC0,
0x60,0x30,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0x7C,0x07,
0x00,0x06,0x00,0x01,0x00,0xC0,0xC0,0xF8,0xCC,0xCC,0xCC,0xF8,0x07,0x00,0x06,0x00,
0x01,0x00,0x00,0x00,0x78,0xCC,0xC0,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x0C,
0x0C,0x7C,0xCC,0xCC,0xCC,0x7C,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0xCC,
0xFC,0xC0,0x7C,0x07,0x00,0x05,0x00,0x01,0x00,0x38,0x60,0xF8,0x60,0x60,0x60,0x60,
0x09,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0xCC,0xCC,0xCC,0x7C,0x0C,0x78,0x07,
0x00,0x06,0x00,0x01,0x00,0xC0,0xC0,0xF8,0xCC,0xCC,0xCC,0xCC,0x07,0x00,0x02,0x00,
0x01,0x00,0xC0,0x00,0xC0,0xC0,0xC0,0xC0,0xC0,0x09,0x00,0x04,0x00,0x01,0x00,0x30,
0x00,0x30,0x30,0x30,0x30,0x30,0x30,0xE0,0x07,0x00,0x06,0x00,0x01,0x00,0xC0,0xC0,
0xCC,0xD8,0xF0,0xD8,0xCC,0x07,0x00,0x02,0x00,0x01,0x00,0xC0,0xC0,0xC0,0xC0,0xC0,
0xC0,0xC0,0x07,0x00,0x08,0x00,0x01,0x00,0x00,0x00,0xFE,0xDB,0xDB,0xDB,0xDB,0x07,
0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xF8,0xCC,0xCC,0xCC,0xCC,0x07,0x00,0x06,0x00,
0x01,0x00,0x00,0x00,0x78,0xCC,0xCC,0xCC,0x78,0x09,0x00,0x06,0x00,0x01,0x00,0x00,
0x00,0xF8,0xCC,0xCC,0xCC,0xF8,0xC0,0xC0,0x09,0x00,0x06,0x00,0x01,0x00,0x00,0x00,
0x78,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,
0xCC,0xC0,0xC0,0xC0,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0xC0,0x78,0x0C,
0xF8,0x07,0x00,0x05,0x00,0x01,0x00,0x60,0x60,0xF8,0x60,0x60,0x60,0x38,0x07,0x00,
0x06,0x00,0x01,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x7C,0x07,0x00,0x06,0x00,0x01,
0x00,0x00,0x00,0xCC,0xCC,0xCC,0x78,0x30,0x07,0x00,0x08,0x00,0x01,0x00,0x00,0x00,
0xC3,0xC3,0xDB,0xFF,0x66,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xCC,0x78,0x30,
0x78,0xCC,0x09,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,
0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xFC,0x18,0x30,0x60,0xFC,0x07,0x00,
0x04,0x00,0x01,0x00,0x30,0x60,0x60,0xC0,0x60,0x60,0x30,0x07,0x00,0x02,0x00,0x01,
0x00,0xC0,0xC0,0xC0,0x00,0xC0,0xC0,0xC0,0x07,0x00,0x04,0x00,0x01,0x00,0xC0,0x60,
0x60,0x30,0x60,0x60,0xC0,0x02,0x00,0x07,0x00,0x01,0x00,0x76,0xDC,0x07,0x00,0x07,
0x00,0x01,0x00,0x00,0x00,0x70,0xC4,0xCC,0x8C,0x38,0x07,0x00,0x07,0x00,0x01,0x00,
0x00,0x06,0x0C,0xD8,0xF0,0xE0,0xC0,0x08,0x00,0x10,0x00,0x02,0x00,0x7F,0xFE,0xCD,
0xC7,0xB5,0xEF,0xB5,0xEF,0x85,0xEF,0xB5,0xEF,0xB4,0x6F,0x08,0x00,0x13,0x00,0x03,
0x00,0x7F,0xFF,0xC0,0xCC,0x46,0xE0,0xB6,0xDA,0xE0,0xBE,0xDA,0xE0,0xBE,0xC6,0xE0,
0xB6,0xDA,0xE0,0xCE,0xDA,0x20,0x7F,0xFF,0xC0,0x08,0x00,0x11,0x00,0x03,0x00,0x7F,
0xFF,0x00,0xC6,0x73,0x80,0xDD,0xAD,0x80,0xCE,0xEF,0x80,0xDF,0x6F,0x80,0xDD,0xAD,
0x80,0xC6,0x73,0x80,0x7F,0xFF,0x00
};

font_type load_font_from_data(/*const*/ rawfont_type* data) {
	font_type font;
	font.first_char = data->first_char;
	font.last_char = data->last_char;
	font.height_above_baseline = data->height_above_baseline;
	font.height_below_baseline = data->height_below_baseline;
	font.space_between_lines = data->space_between_lines;
	font.space_between_chars = data->space_between_chars;
	int n_chars = font.last_char - font.first_char + 1;
	chtab_type* chtab = malloc(sizeof(chtab_type) + sizeof(image_type* far) * n_chars);
	int chr,index;
	// Make a dummy palette for decode_image().
	dat_pal_type dat_pal;
	memset(&dat_pal, 0, sizeof(dat_pal));
	dat_pal.vga[1].r = dat_pal.vga[1].g = dat_pal.vga[1].b = 0x3F; // white
	for (index = 0, chr = data->first_char; chr <= data->last_char; ++index, ++chr) {
		/*const*/ image_data_type* image_data = (/*const*/ image_data_type*)((/*const*/ byte*)data + data->offsets[index]);
		//image_data->flags=0;
		if (image_data->height == 0) image_data->height = 1; // HACK: decode_image() returns NULL if height==0.
		image_type* image;
		chtab->images[index] = image = decode_image(image_data, &dat_pal);
		if (SDL_SetColorKey(image, SDL_TRUE, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	}
	font.chtab = chtab;
	return font;
}

void load_font() {
	// Try to load font from a file.
	dat_type* dathandle = open_dat("font", 0);
	hc_font.chtab = load_sprites_from_file(1000, 1<<1, 0);
	close_dat(dathandle);
	if (hc_font.chtab == NULL) {
		// Use built-in font.
		hc_font = load_font_from_data((/*const*/ rawfont_type*)hc_font_data);
	}
}

// seg009:35C5
int __pascal far get_char_width(byte character) {
	font_type* font = textstate.ptr_font;
	int width = 0;
	if (character <= font->last_char && character >= font->first_char) {
		image_type* image = font->chtab->images[character - font->first_char];
		if (image != NULL) {
			width += image->w; //char_ptrs[character - font->first_char]->width;
			if (width) width += font->space_between_chars;
		}
	}
	return width;
}

// seg009:3E99
int __pascal far find_linebreak(const char far *text,int length,int break_width,int x_align) {
	short curr_line_width; // in pixels
	short last_break_pos; // in characters
	int curr_char_pos = 0;
	last_break_pos = 0;
	curr_line_width = 0;
	const char* text_pos = text;
	while (curr_char_pos < length) {
		curr_line_width += get_char_width(*text_pos);
		if (curr_line_width <= break_width) {
			++curr_char_pos;
			char curr_char = *(text_pos++);
			if (curr_char == '\n') {
				return curr_char_pos;
			}
			if (curr_char == '-' ||
				(x_align <= 0 && (curr_char == ' ' || *text_pos == ' ')) ||
				(*text_pos == ' ' && curr_char == ' ')
			) {
				// May break here.
				last_break_pos = curr_char_pos;
			}
		} else {
			if (last_break_pos == 0) {
				// If the first word is wider than break_width then break it.
				return curr_char_pos;
			} else {
				// Otherwise break at the last space.
				return last_break_pos;
			}
		}
	}
	return curr_char_pos;
}

// seg009:403F
int __pascal far get_line_width(const char far *text,int length) {
	int width = 0;
	const char* text_pos = text;
	while (--length >= 0) {
		width += get_char_width(*(text_pos++));
	}
	return width;
}

// seg009:3706
int __pascal far draw_text_character(byte character) {
	//printf("going to do draw_text_character...\n");
	font_type* font = textstate.ptr_font;
	int width = 0;
	if (character <= font->last_char && character >= font->first_char) {
		image_type* image = font->chtab->images[character - font->first_char]; //char_ptrs[character - font->first_char];
		if (image != NULL) {
			method_3_blit_mono(image, textstate.current_x, textstate.current_y - font->height_above_baseline, textstate.textblit, textstate.textcolor);
			width = font->space_between_chars + image->w;
		}
	}
	textstate.current_x += width;
	return width;
}

// seg009:377F
int __pascal far draw_text_line(const char far *text,int length) {
	//hide_cursor();
	int width = 0;
	const char* text_pos = text;
	while (--length >= 0) {
		width += draw_text_character(*(text_pos++));
	}
	//show_cursor();
	return width;
}

// seg009:3755
int __pascal far draw_cstring(const char far *string) {
	//hide_cursor();
	int width = 0;
	const char* text_pos = string;
	while (*text_pos) {
		width += draw_text_character(*(text_pos++));
	}
	//show_cursor();
	return width;
}

// seg009:3F01
const rect_type far *__pascal draw_text(const rect_type far *rect_ptr,int x_align,int y_align,const char far *text,int length) {
	//printf("going to do draw_text()...\n");
	short rect_top;
	short rect_height;
	short rect_width;
	//textinfo_type var_C;
	short num_lines;
	short font_line_distance;
	//hide_cursor();
	//get_textinfo(&var_C);
	set_clip_rect(rect_ptr);
	rect_width = rect_ptr->right - rect_ptr->left;
	rect_top = rect_ptr->top;
	rect_height = rect_ptr->bottom - rect_ptr->top;
	num_lines = 0;
	int rem_length = length;
	const char* line_start = text;
	#define MAX_LINES 100
	const char* line_starts[MAX_LINES];
	int line_lengths[MAX_LINES];
	do {
		int line_length = find_linebreak(line_start, rem_length, rect_width, x_align);
		if (line_length == 0) break;
		if (num_lines >= MAX_LINES) {
			//... ERROR!
			printf("draw_text(): Too many lines!\n");
			quit(1);
		}
		line_starts[num_lines] = line_start;
		line_lengths[num_lines] = line_length;
		++num_lines;
		line_start += line_length;
		rem_length -= line_length;
	} while(rem_length);
	font_type* font = textstate.ptr_font;
	font_line_distance = font->height_above_baseline + font->height_below_baseline + font->space_between_lines;
	int text_height = font_line_distance * num_lines - font->space_between_lines;
	int text_top = rect_top;
	if (y_align >= 0) {
		if (y_align <= 0) {
			// middle
			// The +1 is for simulating SHR + ADC/SBB.
			text_top += (rect_height+1)/2 - (text_height+1)/2;
		} else {
			// bottom
			text_top += rect_height - text_height;
		}
	}
	textstate.current_y = text_top + font->height_above_baseline;
	int i;
	for (i = 0; i < num_lines; ++i) {
		const char* line_pos = line_starts[i];
		int line_length = line_lengths[i];
		if (x_align < 0 &&
			*line_pos == ' ' &&
			i != 0 &&
			*(line_pos-1) != '\n'
		) {
			// Skip over space if it's not at the beginning of a line.
			++line_pos;
			--line_length;
			if (line_length != 0 &&
				*line_pos == ' ' &&
				*(line_pos-2) == '.'
			) {
				// Skip over second space after point.
				++line_pos;
				--line_length;
			}
		}
		int line_width = get_line_width(line_pos,line_length);
		int text_left = rect_ptr->left;
		if (x_align >= 0) {
			if (x_align <= 0) {
				// center
				text_left += rect_width/2 - line_width/2;
			} else {
				// right
				text_left += rect_width - line_width;
			}
		}
		textstate.current_x = text_left;
		//printf("going to draw text line...\n");
		draw_text_line(line_pos,line_length);
		textstate.current_y += font_line_distance;
	}
	reset_clip_rect();
	//set_textinfo(...);
	//show_cursor();
	return rect_ptr;
}

// seg009:3E4F
void __pascal far show_text(const rect_type far *rect_ptr,int x_align,int y_align,const char far *text) {
	// stub
	//printf("show_text: %s\n",text);
	draw_text(rect_ptr, x_align, y_align, text, strlen(text));
}

// seg009:04FF
void __pascal far show_text_with_color(const rect_type far *rect_ptr,int x_align,int y_align, const char far *text,int color) {
	short saved_textcolor;
	saved_textcolor = textstate.textcolor;
	textstate.textcolor = color;
	show_text(rect_ptr, x_align, y_align, text);
	textstate.textcolor = saved_textcolor;
}

// seg009:3A91
void __pascal far set_curr_pos(int xpos,int ypos) {
	textstate.current_x = xpos;
	textstate.current_y = ypos;
}

// seg009:145A
void __pascal far init_copyprot_dialog() {
	copyprot_dialog = make_dialog_info(&dialog_settings, &dialog_rect_1, &dialog_rect_1, NULL);
	copyprot_dialog->peel = read_peel_from_screen(&copyprot_dialog->peel_rect);
}

// seg009:0838
int __pascal far showmessage(char far *text,int arg_4,void far *arg_0) {
	word key;
	rect_type rect;
	//font_type* saved_font_ptr;
	//surface_type* old_target;
	//old_target = current_target_surface;
	//current_target_surface = onscreen_surface_;
	// In the disassembly there is some messing with the current_target_surface and font (?)
	// However, this does not seem to be strictly necessary
	method_1_blit_rect(offscreen_surface, onscreen_surface_, &copyprot_dialog->peel_rect, &copyprot_dialog->peel_rect, 0);
	draw_dialog_frame(copyprot_dialog);
	//saved_font_ptr = textstate.ptr_font;
	//saved_font_ptr = current_target_surface->ptr_font;
	//current_target_surface->ptr_font = ptr_font;
	shrink2_rect(&rect, &copyprot_dialog->text_rect, 2, 1);
	show_text_with_color(&rect, 0, 0, text, color_15_brightwhite);
	//textstate.ptr_font = saved_font_ptr;
	//current_target_surface->ptr_font = saved_font_ptr;
	clear_kbd_buf();
	do {
		idle();
		key = key_test_quit(); // Press any key to continue...
	} while(key == 0);
	//restore_dialog_peel_2(copyprot_dialog->peel);
	//current_target_surface = old_target;
	need_full_redraw = 1; // lazy: instead of neatly restoring only the relevant part, just redraw the whole screen
	return key;
}

// seg009:08FB
dialog_type * __pascal far make_dialog_info(dialog_settings_type *settings, rect_type *dialog_rect,
                                            rect_type *text_rect, peel_type *dialog_peel) {
	dialog_type* dialog_info;
	dialog_info = malloc_near(sizeof(dialog_type));
	dialog_info->settings = settings;
	dialog_info->has_peel = 0;
	dialog_info->peel = dialog_peel;
	if (text_rect != NULL)
		dialog_info->text_rect = *text_rect;
	calc_dialog_peel_rect(dialog_info);
	if (text_rect != NULL) {        // does not seem to be quite right; see seg009:0948 (?)
		read_dialog_peel(dialog_info);
	}
	return dialog_info;
}

// seg009:0BE7
void __pascal far calc_dialog_peel_rect(dialog_type*dialog) {
	dialog_settings_type* settings;
	settings = dialog->settings;
	dialog->peel_rect.left = dialog->text_rect.left - settings->left_border;
	dialog->peel_rect.top = dialog->text_rect.top - settings->top_border;
	dialog->peel_rect.right = dialog->text_rect.right + settings->right_border + settings->shadow_right;
	dialog->peel_rect.bottom = dialog->text_rect.bottom + settings->bottom_border + settings->shadow_bottom;
}

// seg009:0BB0
void __pascal far read_dialog_peel(dialog_type *dialog) {
	if (dialog->has_peel) {
		if (dialog->peel == NULL) {
			dialog->peel = read_peel_from_screen(&dialog->peel_rect);
		}
		dialog->has_peel = 1;
		draw_dialog_frame(dialog);
	}
}

// seg009:09DE
void __pascal far draw_dialog_frame(dialog_type *dialog) {
	dialog->settings->method_2_frame(dialog);
}

// A pointer to this function is the first field of dialog_settings (data:2944)
// Perhaps used when replacing a dialog's text with another text (?)
// seg009:096F
void __pascal far add_dialog_rect(dialog_type *dialog) {
	draw_rect(&dialog->text_rect, color_0_black);
}

// seg009:09F0
void __pascal far dialog_method_2_frame(dialog_type *dialog) {
	rect_type rect;
	short shadow_right = dialog->settings->shadow_right;
	short shadow_bottom = dialog->settings->shadow_bottom;
	short bottom_border = dialog->settings->bottom_border;
	short outer_border = dialog->settings->outer_border;
	short peel_top = dialog->peel_rect.top;
	short peel_left = dialog->peel_rect.left;
	short peel_bottom = dialog->peel_rect.bottom;
	short peel_right = dialog->peel_rect.right;
	short text_top = dialog->text_rect.top;
	short text_left = dialog->text_rect.left;
	short text_bottom = dialog->text_rect.bottom;
	short text_right = dialog->text_rect.right;
	// Draw outer border
	rect = (rect_type) { peel_top, peel_left, peel_bottom - shadow_bottom, peel_right - shadow_right };
	draw_rect(&rect, color_0_black);
	// Draw shadow (right)
	rect = (rect_type) { text_top, peel_right - shadow_right, peel_bottom, peel_right };
	draw_rect(&rect, get_text_color(0, color_8_darkgray /*dialog's shadow*/, 0));
	// Draw shadow (bottom)
	rect = (rect_type) { peel_bottom - shadow_bottom, text_left, peel_bottom, peel_right };
	draw_rect(&rect, get_text_color(0, color_8_darkgray /*dialog's shadow*/, 0));
	// Draw inner border (left)
	rect = (rect_type) { peel_top + outer_border, peel_left + outer_border, text_bottom, text_left };
	draw_rect(&rect, color_15_brightwhite);
	// Draw inner border (top)
	rect = (rect_type) { peel_top + outer_border, text_left, text_top, text_right + dialog->settings->right_border - outer_border };
	draw_rect(&rect, color_15_brightwhite);
	// Draw inner border (right)
	rect.top = text_top;
	rect.left =  text_right;
	rect.bottom = text_bottom + bottom_border - outer_border;           // (rect.right stays the same)
	draw_rect(&rect, color_15_brightwhite);
	// Draw inner border (bottom)
	rect = (rect_type) { text_bottom, peel_left + outer_border, text_bottom + bottom_border - outer_border, text_right };
	draw_rect(&rect, color_15_brightwhite);
}

// seg009:0C44
void __pascal far show_dialog(const char *text) {
	char string[256];
	snprintf(string, sizeof(string), "%s\n\nPress any key to continue.", text);
	showmessage(string, 1, &key_test_quit);
}

// seg009:0791
int __pascal far get_text_center_y(const rect_type far *rect) {
	const font_type far* font;
	short empty_height; // height of empty space above+below the line of text
	font = &hc_font;//current_target_surface->ptr_font;
	empty_height = rect->bottom - font->height_above_baseline - font->height_below_baseline - rect->top;
	return ((empty_height - empty_height % 2) >> 1) + font->height_above_baseline + empty_height % 2 + rect->top;
}

// seg009:3E77
int __pascal far get_cstring_width(const char far *text) {
	int width = 0;
	const char* text_pos = text;
	char curr_char;
	while (0 != (curr_char = *(text_pos++))) {
		width += get_char_width(curr_char);
	}
	return width;
}

// seg009:0767
void __pascal far draw_text_cursor(int xpos,int ypos,int color) {
	set_curr_pos(xpos, ypos);
	/*current_target_surface->*/textstate.textcolor = color;
	draw_text_character('_');
	//restore_curr_color();
	textstate.textcolor = 15;
}

// seg009:053C
int __pascal far input_str(const rect_type far *rect,char *buffer,int max_length,const char *initial,int has_initial,int arg_4,int color,int bgcolor) {
	short length;
	word key;
	short cursor_visible;
	short current_xpos;
	short ypos;
	short init_length;
	length = 0;
	cursor_visible = 0;
	draw_rect(rect, bgcolor);
	init_length = strlen(initial);
	if (has_initial) {
		strcpy(buffer, initial);
		length = init_length;
	}
	current_xpos = rect->left + arg_4;
	ypos = get_text_center_y(rect);
	set_curr_pos(current_xpos, ypos);
	/*current_target_surface->*/textstate.textcolor = color;
	draw_cstring(initial);
	//restore_curr_pos?();
	current_xpos += get_cstring_width(initial) + (init_length != 0) * arg_4;
	do {
		key = 0;
		do {
			if (cursor_visible) {
				draw_text_cursor(current_xpos, ypos, color);
			} else {
				draw_text_cursor(current_xpos, ypos, bgcolor);
			}
			cursor_visible = !cursor_visible;
			start_timer(timer_0, 6);
			if (key) {
				if (cursor_visible) {
					draw_text_cursor(current_xpos, ypos, color);
					cursor_visible = !cursor_visible;
				}
				if (key == SDL_SCANCODE_RETURN) { // enter
					buffer[length] = 0;
					return length;
				} else break;
			}
			while (!has_timer_stopped(timer_0) && (key = key_test_quit()) == 0) idle();
		} while (1);
		// Only use the printable ASCII chars (UTF-8 encoding)
		char entered_char = last_text_input <= 0x7E ? last_text_input : 0;
		clear_kbd_buf();

		if (key == SDL_SCANCODE_ESCAPE) { // esc
			draw_rect(rect, bgcolor);
			buffer[0] = 0;
			return -1;
		}
		if (length != 0 && (key == SDL_SCANCODE_BACKSPACE ||
				key == SDL_SCANCODE_DELETE)) { // backspace, delete
			--length;
			draw_text_cursor(current_xpos, ypos, bgcolor);
			current_xpos -= get_char_width(buffer[length]);
			set_curr_pos(current_xpos, ypos);
			/*current_target_surface->*/textstate.textcolor = bgcolor;
			draw_text_character(buffer[length]);
			//restore_curr_pos?();
			draw_text_cursor(current_xpos, ypos, color);
		}
		else if (entered_char >= 0x20 && entered_char <= 0x7E && length < max_length) {
			// Would the new character make the cursor go past the right side of the rect?
			if (get_char_width('_') + get_char_width(entered_char) + current_xpos < rect->right) {
				draw_text_cursor(current_xpos, ypos, bgcolor);
				set_curr_pos(current_xpos, ypos);
				/*current_target_surface->*/textstate.textcolor = color;
				current_xpos += draw_text_character(buffer[length++] = entered_char);
			}
		}
	} while(1);
}

#else // USE_TEXT

// seg009:3706
int __pascal far draw_text_character(byte character) {
	// stub
	printf("draw_text_character: %c\n",character);
	return 0;
}

// seg009:3E4F
void __pascal far show_text(const rect_type far *rect_ptr,int x_align,int y_align,const char far *text) {
	// stub
	printf("show_text: %s\n",text);
}

// seg009:04FF
void __pascal far show_text_with_color(const rect_type far *rect_ptr,int x_align,int y_align,char far *text,int color) {
	//short saved_textcolor;
	//saved_textcolor = textstate.textcolor;
	//textstate.textcolor = color;
	show_text(rect_ptr, x_align, y_align, text);
	//textstate.textcolor = saved_textcolor;
}

// seg009:3A91
void __pascal far set_curr_pos(int xpos,int ypos) {
	// stub
}

// seg009:0C44
void __pascal far show_dialog(char *text) {
	// stub
	puts(text);
}

// seg009:053C
int __pascal far input_str(const rect_type far *rect,char *buffer,int max_length,const char *initial,int has_initial,int arg_4,int color,int bgcolor) {
	// stub
	strncpy(buffer, "dummy input text", max_length);
	return strlen(buffer);
}

#endif // USE_TEXT

// seg009:37E8
void __pascal far draw_rect(const rect_type far *rect,int color) {
	method_5_rect(rect, blitters_0_no_transp, color);
}

// seg009:3985
surface_type far *__pascal rect_sthg(surface_type *surface,const rect_type far *rect) {
	// stub
	return surface;
}

// seg009:39CE
rect_type far *__pascal shrink2_rect(rect_type far *target_rect,const rect_type far *source_rect,int delta_x,int delta_y) {
	target_rect->top    = source_rect->top    + delta_y;
	target_rect->left   = source_rect->left   + delta_x;
	target_rect->bottom = source_rect->bottom - delta_y;
	target_rect->right  = source_rect->right  - delta_x;
	return target_rect;
}

// seg009:3BBA
void __pascal far restore_peel(peel_type* peel_ptr) {
	//printf("restoring peel at (x=%d, y=%d)\n", peel_ptr.rect.left, peel_ptr.rect.top); // debug
	method_6_blit_img_to_scr(peel_ptr->peel, peel_ptr->rect.left, peel_ptr->rect.top, /*0x10*/0);
	free_peel(peel_ptr);
	//SDL_FreeSurface(peel_ptr.peel);
}

// seg009:3BE9
peel_type* __pascal far read_peel_from_screen(const rect_type far *rect) {
	// stub
	peel_type* result;
	result = calloc(1, sizeof(peel_type));
	//memset(&result, 0, sizeof(result));
	result->rect = *rect;
#ifndef USE_ALPHA
	SDL_Surface* peel_surface = SDL_CreateRGBSurface(0, rect->right - rect->left, rect->bottom - rect->top,
	                                                 24, 0xFF, 0xFF<<8, 0xFF<<16, 0);
#else
	SDL_Surface* peel_surface = SDL_CreateRGBSurface(0, rect->right - rect->left, rect->bottom - rect->top, 32, 0xFF, 0xFF<<8, 0xFF<<16, 0xFF<<24);
#endif
	if (peel_surface == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	result->peel = peel_surface;
	rect_type target_rect = {0, 0, rect->right - rect->left, rect->bottom - rect->top};
	method_1_blit_rect(result->peel, current_target_surface, &target_rect, rect, 0);
	return result;
}

// seg009:3D95
int __pascal far intersect_rect(rect_type far *output,const rect_type far *input1,const rect_type far *input2) {
	short left = MAX(input1->left, input2->left);
	short right = MIN(input1->right, input2->right);
	if (left < right) {
		output->left = left;
		output->right = right;
		short top = MAX(input1->top, input2->top);
		short bottom = MIN(input1->bottom, input2->bottom);
		if (top < bottom) {
			output->top = top;
			output->bottom = bottom;
			return 1;
		}
	}
	memset(output, 0, sizeof(rect_type));
	return 0;
}

// seg009:4063
rect_type far * __pascal far union_rect(rect_type far *output,const rect_type far *input1,const rect_type far *input2) {
	short top = MIN(input1->top, input2->top);
	short left = MIN(input1->left, input2->left);
	short bottom = MAX(input1->bottom, input2->bottom);
	short right = MAX(input1->right, input2->right);
	output->top = top;
	output->left = left;
	output->bottom = bottom;
	output->right = right;
	return output;
}

enum userevents {
	userevent_SOUND,
	userevent_TIMER,
};

SDL_TimerID sound_timer = 0;
short speaker_playing = 0;
short digi_playing = 0;
short midi_playing = 0;

void __pascal far speaker_sound_stop() {
	// stub
	speaker_playing = 0;
	if (sound_timer != 0) {
		if (!SDL_RemoveTimer(sound_timer)) {
			sdlperror("SDL_RemoveTimer in speaker_sound_stop");
			//quit(1);
		}
		sound_timer = 0;
	}
}

// The current buffer, holds the resampled sound data.
byte* digi_buffer = NULL;
// The current position in digi_buffer.
byte* digi_remaining_pos = NULL;
// The remaining length.
size_t digi_remaining_length = 0;

// The properties of the audio device.
SDL_AudioSpec* digi_audiospec = NULL;
// The desired samplerate. Everything will be resampled to this.
const int digi_samplerate = 44100;

void stop_digi() {
#ifndef USE_MIXER
	SDL_PauseAudio(1);
	if (!digi_playing) return;
	SDL_LockAudio();
	digi_playing = 0;
	/*
//	if (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING) {
		SDL_PauseAudio(1);
		SDL_CloseAudio();
//	}
	if (digi_audiospec != NULL) {
		free(digi_audiospec);
		digi_audiospec = NULL;
	}
	*/
	if (digi_buffer != NULL) {
		free(digi_buffer);
		digi_buffer = NULL;
	}
	digi_remaining_length = 0;
	digi_remaining_pos = NULL;
	SDL_UnlockAudio();
#else
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
	digi_playing = 0;
#endif
}

// seg009:7214
void __pascal far stop_sounds() {
	// stub
	stop_digi();
	// stop_midi();
	speaker_sound_stop();
}

Uint32 speaker_callback(Uint32 interval, void *param) {
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_USEREVENT;
	event.user.code = userevent_SOUND;
	event.user.data1 = param;
	if (!SDL_RemoveTimer(sound_timer)) {
		sdlperror("SDL_RemoveTimer in speaker_callback");
		//quit(1);
	}
	sound_timer = 0;
	speaker_playing = 0;
	// First remove the timer, then allow the other thread to continue.
	SDL_PushEvent(&event);
	return 0;
}

// seg009:7640
void __pascal far play_speaker_sound(sound_buffer_type far *buffer) {
	// stub
	//speaker_sound_stop();
	stop_sounds();
	int length = 0;
	int index;
	for (index = 0; buffer->speaker.notes[index].frequency != 0x12; ++index) {
		length += buffer->speaker.notes[index].length;
	}
	int time_ms = length*1000 / buffer->speaker.tempo;
	//printf("length = %d ms\n", time_ms);
	sound_timer = SDL_AddTimer(time_ms, speaker_callback, NULL);
	if (sound_timer == 0) {
		sdlperror("SDL_AddTimer");
		quit(1);
	}
	speaker_playing = 1;
}

#ifndef USE_MIXER
void digi_callback(void *userdata, Uint8 *stream, int len) {
	// Don't go over the end of either the input or the output buffer.
	size_t copy_len = MIN(len, digi_remaining_length);
	//printf("digi_callback(): copy_len = %d\n", copy_len);
	//printf("digi_callback(): len = %d\n", len);
	if (is_sound_on) {
		// Copy the next part of the input of the output.
		memcpy(stream, digi_remaining_pos, copy_len);
		// In case the sound does not fill the buffer: fill the rest of the buffer with silence.
		memset(stream + copy_len, digi_audiospec->silence, len - copy_len);
	} else {
		// If sound is off: Mute the sound but keep track of where we are.
		memset(stream, digi_audiospec->silence, len);
	}
	// If the sound ended, push an event.
	if (digi_playing && digi_remaining_length == 0) {
		//printf("digi_callback(): sound ended\n");
		SDL_Event event;
		memset(&event, 0, sizeof(event));
		event.type = SDL_USEREVENT;
		event.user.code = userevent_SOUND;
		digi_playing = 0;
		SDL_PushEvent(&event);
	}
	// Advance the pointer.
	digi_remaining_length -= copy_len;
	digi_remaining_pos += copy_len;
}
#endif

#ifdef USE_MIXER
void channel_finished(int channel) {
	digi_playing = 0;
	//printf("Finished channel %d\n", channel);
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_USEREVENT;
	event.user.code = userevent_SOUND;
	SDL_PushEvent(&event);
}
void music_finished(void) {
	channel_finished(-1);
}
#endif

int digi_unavailable = 0;
void init_digi() {
	if (digi_unavailable) return;
	if (digi_audiospec != NULL) return;
	// Open the audio device. Called once.
	//printf("init_digi(): called\n");

	SDL_AudioFormat desired_audioformat;
	SDL_version version;
	SDL_GetVersion(&version);
	//printf("SDL Version = %d.%d.%d\n", version.major, version.minor, version.patch);
	if (version.major <= 2 && version.minor <= 0 && version.patch <= 3) {
		// In versions before 2.0.4, 16-bit audio samples don't work properly (the sound becomes garbled).
		// See: https://bugzilla.libsdl.org/show_bug.cgi?id=2389
		// Workaround: set the audio format to 8-bit, if we are linking against an older SDL2 version.
		desired_audioformat = AUDIO_U8;
		printf("Your SDL.dll is older than 2.0.4. Using 8-bit audio format to work around resampling bug.");
	} else {
		desired_audioformat = AUDIO_S16SYS;
	}

	SDL_AudioSpec *desired;
	desired = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
	memset(desired, 0, sizeof(SDL_AudioSpec));
	desired->freq = digi_samplerate; //buffer->digi.sample_rate;
	desired->format = desired_audioformat;
	desired->channels = 2;
	desired->samples = 1024;
#ifndef USE_MIXER
	desired->callback = digi_callback;
	desired->userdata = NULL;
	if (SDL_OpenAudio(desired, NULL) != 0) {
		sdlperror("SDL_OpenAudio");
		//quit(1);
		digi_unavailable = 1;
		return;
	}
	//SDL_PauseAudio(0);
#else
	if (Mix_OpenAudio(desired->freq, desired->format, desired->channels, desired->samples) != 0) {
		sdlperror("Mix_OpenAudio");
		digi_unavailable = 1;
		return;
	}
	Mix_AllocateChannels(1);
	Mix_ChannelFinished(channel_finished);
	Mix_HookMusicFinished(music_finished);
#endif
	digi_audiospec = desired;
}

#ifdef USE_MIXER
const int sound_channel = 0;
const int max_sound_id = 58;
char** sound_names = NULL;

void load_sound_names() {
	const char* names_path = "data/music/names.txt";
	if (sound_names != NULL) return;
	FILE* fp = fopen(names_path,"rt");
	if (fp==NULL) return;
	sound_names = (char**) calloc(sizeof(char*) * max_sound_id, 1);
	while (!feof(fp)) {
		int index;
		char name[POP_MAX_PATH];
		if (fscanf(fp, "%d=%255s\n", &index, /*sizeof(name)-1,*/ name) != 2) {
			perror(names_path);
			continue;
		}
		//if (feof(fp)) break;
		//printf("sound_names[%d] = %s\n",index,name);
		if (index >= 0 && index < max_sound_id) {
			sound_names[index] = strdup(name);
		}
	}
	fclose(fp);
}

char* sound_name(int index) {
	if (sound_names != NULL && index >= 0 && index < max_sound_id) {
		return sound_names[index];
	} else {
		return NULL;
	}
}

void convert_digi_sound(sound_buffer_type *buffer);
#endif

sound_buffer_type* load_sound(int index) {
	sound_buffer_type* result = NULL;
#ifdef USE_MIXER
	//printf("load_sound(%d)\n", index);
	init_digi();
	if (!digi_unavailable && result == NULL && index >= 0 && index < max_sound_id) {
		//printf("Trying to load from music folder\n");

		//load_sound_names();  // Moved to load_sounds()
		if (sound_names != NULL && sound_name(index) != NULL) {
			//printf("Loading from music folder\n");
			const char* exts[]={"ogg","mp3","flac","wav"};
			int i;
			for (i = 0; i < COUNT(exts); ++i) {
				char filename[POP_MAX_PATH];
				const char* ext=exts[i];
				struct stat info;

				snprintf(filename, sizeof(filename), "data/music/%s.%s", sound_name(index), ext);
				// Skip nonexistent files:
				if (stat(filename, &info))
					continue;
				//printf("Trying to load %s\n", filename);
				Mix_Music* music = Mix_LoadMUS(filename);
				if (music == NULL) {
					sdlperror(filename);
					//sdlperror("Mix_LoadWAV");
					continue;
				}
				//printf("Loaded sound from %s\n", filename);
				result = malloc(sizeof(sound_buffer_type));
				result->type = sound_music;
				result->music = music;
				break;
			}
		} else {
			//printf("sound_names = %p\n", sound_names);
			//printf("sound_names[%d] = %p\n", index, sound_name(index));
		}
	}
#endif
	if (result == NULL) {
		//printf("Trying to load from DAT\n");
		result = (sound_buffer_type*) load_from_opendats_alloc(index + 10000, "bin", NULL, NULL);
	}
#ifdef USE_MIXER
	if (result == NULL) {
		fprintf(stderr, "Failed to load sound %d '%s'\n", index, sound_name(index));
	} else {
		// Convert waves to mixer chunks in advance.
		if ((result->type & 7) == sound_digi) {
			convert_digi_sound(result);
		}
	}
#endif
	return result;
}

#ifdef USE_MIXER
void __pascal far play_chunk_sound(sound_buffer_type far *buffer) {
	//if (!is_sound_on) return;
	init_digi();
	if (digi_unavailable) return;
	stop_sounds();
	//printf("playing chunk sound %p\n", buffer);
	if (Mix_PlayChannel(sound_channel, buffer->chunk, 0) == -1) {
		sdlperror("Mix_PlayChannel");
	}
	digi_playing = 1;
}

void __pascal far play_music_sound(sound_buffer_type far *buffer) {
	init_digi();
	if (digi_unavailable) return;
	stop_sounds();
	if (Mix_PlayMusic(buffer->music, 0) == -1) {
		sdlperror("Mix_PlayMusic");
	}
	digi_playing = 1;
}

Uint32 fourcc(char* string) {
	return *(Uint32*)string;
}
#endif

int wave_version = -1;

typedef struct waveinfo_type {
	int sample_rate, sample_size, sample_count;
	byte* samples;
} waveinfo_type;

bool determine_wave_version(sound_buffer_type *buffer, waveinfo_type* waveinfo);

bool determine_wave_version(sound_buffer_type *buffer, waveinfo_type* waveinfo) {
	int version = wave_version;
	if (version == -1) {
		// Determine the version of the wave data.
		version = 0;
		if (buffer->digi.sample_size == 8) version += 1;
		if (buffer->digi_new.sample_size == 8) version += 2;
		if (version == 1 || version == 2) wave_version = version;
	}

	switch (version) {
		case 1: // 1.0 and 1.1
			waveinfo->sample_rate = buffer->digi.sample_rate;
			waveinfo->sample_size = buffer->digi.sample_size;
			waveinfo->sample_count = buffer->digi.sample_count;
			waveinfo->samples = buffer->digi.samples;
			return true;
		case 2: // 1.3 and 1.4 (and PoP2)
			waveinfo->sample_rate = buffer->digi_new.sample_rate;
			waveinfo->sample_size = buffer->digi_new.sample_size;
			waveinfo->sample_count = buffer->digi_new.sample_count;
			waveinfo->samples = buffer->digi_new.samples;
			return true;
		case 3: // ambiguous
			printf("Warning: Ambiguous wave version.\n");
			return false;
		default: // case 0, unknown
			printf("Warning: Can't determine wave version.\n");
			return false;
	}
}

#ifndef USE_MIXER
// seg009:74F0
void __pascal far play_digi_sound(sound_buffer_type far *buffer) {
	//if (!is_sound_on) return;
	init_digi();
	if (digi_unavailable) return;
	//stop_digi();
	stop_sounds();
	//printf("play_digi_sound(): called\n");

	waveinfo_type waveinfo;
	if (false == determine_wave_version(buffer, &waveinfo)) return;

	SDL_AudioCVT cvt;
	memset(&cvt, 0, sizeof(cvt));
	int result = SDL_BuildAudioCVT(&cvt,
		AUDIO_U8, 1, waveinfo.sample_rate,
		digi_audiospec->format, digi_audiospec->channels, digi_audiospec->freq
	);
	// The case of result == 0 is undocumented, but it may occur.
	if (result != 1 && result != 0) {
		sdlperror("SDL_BuildAudioCVT");
		printf("(returned %d)\n", result);
		quit(1);
	}
	int dlen = waveinfo.sample_count; // if format is AUDIO_U8
	cvt.buf = (Uint8*) malloc(dlen * cvt.len_mult);
	memcpy(cvt.buf, waveinfo.samples, dlen);
	cvt.len = dlen;
	if (SDL_ConvertAudio(&cvt) != 0) {
		sdlperror("SDL_ConvertAudio");
		quit(1);
	}

	SDL_LockAudio();
	digi_buffer = cvt.buf;
	digi_playing = 1;
//	digi_remaining_length = sample_count;
//	digi_remaining_pos = samples;
	digi_remaining_length = cvt.len_cvt;
	digi_remaining_pos = digi_buffer;
	SDL_UnlockAudio();
	SDL_PauseAudio(0);
}
#else
void __pascal far play_digi_sound(sound_buffer_type far *buffer) {
	printf("Warning: Tried to play a digi sound without converting it to a mixer chunk first!\n");
}

void convert_digi_sound(sound_buffer_type *buffer) {
	waveinfo_type waveinfo;
	if (false == determine_wave_version(buffer, &waveinfo)) return;

	// Convert the DAT sound to WAV, so the Mixer can load it.
	int size = waveinfo.sample_count;
	int rounded_size = (size+1)&(~1);
	int alloc_size = sizeof(WAV_header_type) + rounded_size;
	WAV_header_type* wav_data = malloc(alloc_size);
	wav_data->ChunkID = fourcc("RIFF");
	wav_data->ChunkSize = 36 + rounded_size;
	wav_data->Format = fourcc("WAVE");
	wav_data->Subchunk1ID = fourcc("fmt ");
	wav_data->Subchunk1Size = 16;
	wav_data->AudioFormat = 1; // PCM
	wav_data->NumChannels = 1; // Mono
	wav_data->SampleRate = waveinfo.sample_rate;
	wav_data->BitsPerSample = waveinfo.sample_size;
	wav_data->ByteRate = wav_data->SampleRate * wav_data->NumChannels * wav_data->BitsPerSample/8;
	wav_data->BlockAlign = wav_data->NumChannels * wav_data->BitsPerSample/8;
	wav_data->Subchunk2ID = fourcc("data");
	wav_data->Subchunk2Size = size;
	memcpy(wav_data->Data, waveinfo.samples, size);
	SDL_RWops* rw = SDL_RWFromConstMem(wav_data, alloc_size);
	Mix_Chunk *chunk = Mix_LoadWAV_RW(rw, 1);
	if (chunk == NULL) {
		FILE* fp = fopen("dump.wav","wb");
		fwrite(wav_data,alloc_size,1,fp);
		fclose(fp);
	}
	free(wav_data);
	if (chunk == NULL) {
		sdlperror("Mix_LoadWAV_RW");
		return;
	}
	buffer->type = sound_chunk;
	buffer->chunk = chunk;
}
#endif

void free_sound(sound_buffer_type far *buffer) {
	if (buffer == NULL) return;
#ifdef USE_MIXER
	if (buffer->type == sound_chunk) {
		Mix_FreeChunk(buffer->chunk);
	}
	if (buffer->type == sound_music) {
		Mix_FreeMusic(buffer->music);
	}
#endif
	free(buffer);
}

// seg009:7220
void __pascal far play_sound_from_buffer(sound_buffer_type far *buffer) {

#ifdef USE_REPLAY
	if (replaying && skipping_replay) return;
#endif

	// stub
	if (buffer == NULL) {
		printf("Tried to play NULL sound.\n");
		//quit(1);
		return;
	}
	switch (buffer->type & 7) {
		case sound_speaker:
			play_speaker_sound(buffer);
		break;
		case sound_digi:
			play_digi_sound(buffer);
		break;
#ifdef USE_MIXER
		case sound_chunk:
			play_chunk_sound(buffer);
		break;
		case sound_music:
			play_music_sound(buffer);
		break;
#endif
		default:
			printf("Tried to play unimplemented sound type %d.\n", buffer->type);
			quit(1);
		break;
	}
}

// seg009:7273
void __pascal far turn_sound_on_off(byte new_state) {
	// stub
	is_sound_on = new_state;
	//if (!is_sound_on) stop_sounds();
#ifdef USE_MIXER
	init_digi();
	if (digi_unavailable) return;
	Mix_Volume(-1, is_sound_on ? MIX_MAX_VOLUME : 0);
	Mix_VolumeMusic(is_sound_on ? MIX_MAX_VOLUME : 0);
#endif
}

// seg009:7299
int __pascal far check_sound_playing() {
	return speaker_playing || digi_playing || midi_playing;
}

float get_display_refresh_time() {
	SDL_DisplayMode display_mode;
	float refresh_time;
	if (SDL_GetWindowDisplayMode(window_, &display_mode) == 0 && display_mode.refresh_rate > 0) {
		refresh_time = 1000.0f / (float)display_mode.refresh_rate;
	} else {
		refresh_time = 1000.0f / 60.0f;
	}
	return refresh_time;
}

bool checked_vsync;
bool vsync_enabled;

void detect_vsync() {
	if (!checked_vsync) {
		SDL_RenderPresent(renderer_);
		Uint64 start_counter = SDL_GetPerformanceCounter();
		SDL_RenderPresent(renderer_);
		SDL_RenderPresent(renderer_);
		SDL_RenderPresent(renderer_);
		SDL_RenderPresent(renderer_);
		float time_elapsed = (SDL_GetPerformanceCounter() - start_counter) * milliseconds_per_counter;
		float refresh_time = get_display_refresh_time();
		if (time_elapsed > 3.0f * refresh_time) {
			vsync_enabled = true;
		}
		checked_vsync = true;
	}
}

// seg009:38ED
void __pascal far set_gr_mode(byte grmode) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE |
	             SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC ) != 0) {
		sdlperror("SDL_Init");
		quit(1);
	}

	//SDL_EnableUNICODE(1); //deprecated
	Uint32 flags = 0;
	if (!start_fullscreen) start_fullscreen = check_param("full") != NULL;
	if (start_fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	flags |= SDL_WINDOW_RESIZABLE;

	// Should use different default window dimensions when using 4:3 aspect ratio
	if (use_correct_aspect_ratio && pop_window_width == 640 && pop_window_height == 400) {
		pop_window_height = 480;
	}

#ifdef USE_REPLAY
	if (!is_validate_mode) // run without a window if validating a replay
#endif
	window_ = SDL_CreateWindow(WINDOW_TITLE,
	                           SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                           pop_window_width, pop_window_height, flags);
	renderer_ = SDL_CreateRenderer(window_, -1 , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_Surface* icon = IMG_Load("data/icon.png");
	if (icon == NULL) {
		sdlperror("Could not load icon");
	} else {
		SDL_SetWindowIcon(window_, icon);
	}

	// Allow us to use a consistent set of screen co-ordinates, even if the screen size changes
	if (use_correct_aspect_ratio) {
		SDL_RenderSetLogicalSize(renderer_, 320*5, 200*6);
	} else {
		SDL_RenderSetLogicalSize(renderer_, 320, 200);
	}

	/* Migration to SDL2: everything is still blitted to onscreen_surface_, however:
	 * SDL2 renders textures to the screen instead of surfaces; so for now, every screen
	 * update causes the onscreen_surface_ to be copied into sdl_texture_, which is
	 * subsequently displayed; awaits a better refactoring!
	 * The function handling the screen updates is update_screen()
	 * */
	onscreen_surface_ = SDL_CreateRGBSurface(0, 320, 200, 24, 0xFF, 0xFF << 8, 0xFF << 16, 0) ;
	overlay_surface = SDL_CreateRGBSurface(0, 320, 200, 32, 0xFF, 0xFF << 8, 0xFF << 16, 0xFF << 24) ;
	merged_surface = SDL_CreateRGBSurface(0, 320, 200, 24, 0xFF, 0xFF << 8, 0xFF << 16, 0) ;
	sdl_texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 320, 200);

	if (onscreen_surface_ == NULL || overlay_surface == NULL || merged_surface == NULL) {
		sdlperror("SDL_SetVideoMode");
		quit(1);
	}
	if (start_fullscreen) {
		SDL_ShowCursor(SDL_DISABLE);
	}

	milliseconds_per_counter = 1000.0f / SDL_GetPerformanceFrequency();

	//SDL_WM_SetCaption(WINDOW_TITLE, NULL);
//	if (SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL) != 0) {  //deprecated
//		sdlperror("SDL_EnableKeyRepeat");
//		quit(1);
//	}
	graphics_mode = gmMcgaVga;
#ifdef USE_TEXT
	load_font();
#endif
}

void draw_overlays() {
	surface_type* saved_target_surface = current_target_surface;
	current_target_surface = overlay_surface;
	SDL_FillRect(overlay_surface, NULL, 0);

#ifdef USE_DEBUG_CHEATS
	if (debug_cheats_enabled && is_timer_displayed && start_level > 0) {
		char timer_text[32];
		if (rem_min < 0) {
			snprintf(timer_text, sizeof(timer_text), "%02d:%02d:%02d",
			         -(rem_min + 1), (719 - rem_tick) / 12, (719 - rem_tick) % 12);
		} else {
			snprintf(timer_text, sizeof(timer_text), "%02d:%02d:%02d",
			         rem_min - 1, rem_tick / 12, rem_tick % 12);
		}
		int expected_numeric_chars = 6;
		int extra_numeric_chars = MAX(0, strnlen(timer_text, sizeof(timer_text)) - 8);
		int line_width = 5 + (expected_numeric_chars + extra_numeric_chars) * 9;

		rect_type timer_box_rect = {0, 0, 11, 2 + line_width};
		rect_type timer_text_rect = {2, 2, 10, 100};
		draw_rect_with_alpha(&timer_box_rect, color_0_black, 128);
		show_text(&timer_text_rect, -1, -1, timer_text);
	}
#endif

	current_target_surface = saved_target_surface;
}

void update_screen() {
	draw_overlays();

	SDL_FillRect(merged_surface, NULL, 0);
	SDL_BlitSurface(onscreen_surface_, NULL, merged_surface, NULL);
	SDL_BlitSurface(overlay_surface, NULL, merged_surface, NULL);

	SDL_UpdateTexture(sdl_texture_, NULL, merged_surface->pixels, merged_surface->pitch);
	SDL_RenderClear(renderer_);
	SDL_RenderCopy(renderer_, sdl_texture_, NULL, NULL);
	SDL_RenderPresent(renderer_);
}

// seg009:9289
void __pascal far set_pal_arr(int start,int count,const rgb_type far *array,int vsync) {
	// stub
	int i;
	for (i = 0; i < count; ++i) {
		if (array) {
			set_pal(start + i, array[i].r, array[i].g, array[i].b, vsync);
		} else {
			set_pal(start + i, 0, 0, 0, vsync);
		}
	}
}

rgb_type palette[256];

// seg009:92DF
void __pascal far set_pal(int index,int red,int green,int blue,int vsync) {
	// stub
	//palette[index] = ((red&0x3F)<<2)|((green&0x3F)<<2<<8)|((blue&0x3F)<<2<<16);
	palette[index].r = red;
	palette[index].g = green;
	palette[index].b = blue;
}

// seg009:969C
int __pascal far add_palette_bits(byte n_colors) {
	// stub
	return 0;
}

// seg009:9C36
int __pascal far find_first_pal_row(int which_rows_mask) {
	word which_row = 0;
	word row_mask = 1;
	do {
		if (row_mask & which_rows_mask) {
			return which_row;
		}
		++which_row;
		row_mask <<= 1;
	} while (which_row < 16);
	return 0;
}

// seg009:9C6C
int __pascal far get_text_color(int cga_color,int low_half,int high_half_mask) {
	if (graphics_mode == gmCga || graphics_mode == gmHgaHerc) {
		return cga_color;
	} else if (graphics_mode == gmMcgaVga && high_half_mask != 0) {
		return (find_first_pal_row(high_half_mask) << 4) + low_half;
	} else {
		return low_half;
	}
}

void load_from_opendats_metadata(int resource_id, const char* extension, FILE** out_fp, data_location* result, byte* checksum, int* size, dat_type** out_pointer) {
	char image_filename[POP_MAX_PATH];
	FILE* fp = NULL;
	dat_type* pointer;
	*result = data_none;
	// Go through all open DAT files.
	for (pointer = dat_chain_ptr; fp == NULL && pointer != NULL; pointer = pointer->next_dat) {
		*out_pointer = pointer;
		if (pointer->handle != NULL) {
			// If it's an actual DAT file:
			fp = pointer->handle;
			dat_table_type* dat_table = pointer->dat_table;
			int i;
			for (i = 0; i < dat_table->res_count; ++i) {
				if (dat_table->entries[i].id == resource_id) {
					break;
				}
			}
			if (i < dat_table->res_count) {
				// found
				*result = data_DAT;
				*size = dat_table->entries[i].size;
				if (fseek(fp, dat_table->entries[i].offset, SEEK_SET) ||
				    fread(checksum, 1, 1, fp) != 1) {
					perror(pointer->filename);
					fp = NULL;
				}
			} else {
				// not found
				fp = NULL;
			}
		} else {
			// If it's a directory:
			char filename_no_ext[POP_MAX_PATH];
			// strip the .DAT file extension from the filename (use folders simply named TITLE, KID, VPALACE, etc.)
			strncpy(filename_no_ext, pointer->filename, sizeof(filename_no_ext));
			size_t len = strlen(filename_no_ext);
			if (len >= 5 && filename_no_ext[len-4] == '.') {
				filename_no_ext[len-4] = '\0'; // terminate, so ".DAT" is deleted from the filename
			}
			snprintf(image_filename,sizeof(image_filename),"data/%s/res%d.%s",filename_no_ext, resource_id, extension);
			if (!use_custom_levelset) {
				//printf("loading (binary) %s",image_filename);
				fp = fopen(image_filename, "rb");
			}
			else {
				char image_filename_mod[POP_MAX_PATH];
				// before checking data/, first try mods/MODNAME/data/
				snprintf(image_filename_mod, sizeof(image_filename_mod), "mods/%s/%s", levelset_name, image_filename);
				//printf("loading (binary) %s",image_filename_mod);
				fp = fopen(image_filename_mod, "rb");
				if (fp == NULL) {
					fp = fopen(image_filename, "rb");
				}
			}

			if (fp != NULL) {
				struct stat buf;
				if (fstat(fileno(fp), &buf) == 0) {
					*result = data_directory;
					*size = buf.st_size;
				} else {
					perror(image_filename);
					fclose(fp);
					fp = NULL;
				}
			}
		}
	}
	*out_fp = fp;
	if (fp == NULL) {
		*result = data_none;
//		printf(" FAILED\n");
		//return NULL;
	}
	//...
}

// seg009:9F34
void __pascal far close_dat(dat_type far *pointer) {
	dat_type** prev = &dat_chain_ptr;
	dat_type* curr = dat_chain_ptr;
	while (curr != NULL) {
		if (curr == pointer) {
			*prev = curr->next_dat;
			if (curr->handle) fclose(curr->handle);
			if (curr->dat_table) free(curr->dat_table);
			free(curr);
			return;
		}
		curr = curr->next_dat;
		prev = &((*prev)->next_dat);
	}
	// stub
}

// seg009:9F80
void far *__pascal load_from_opendats_alloc(int resource, const char* extension, data_location* out_result, int* out_size) {
	// stub
	//printf("id = %d\n",resource);
	dat_type* pointer;
	data_location result;
	byte checksum;
	int size;
	FILE* fp = NULL;
	load_from_opendats_metadata(resource, extension, &fp, &result, &checksum, &size, &pointer);
	if (out_result != NULL) *out_result = result;
	if (out_size != NULL) *out_size = size;
	if (result == data_none) return NULL;
	void* area = malloc(size);
	//read(fd, area, size);
	if (fread(area, size, 1, fp) != 1) {
		fprintf(stderr, "%s: %s, resource %d, size %d, failed: %s\n",
			__func__, pointer->filename, resource,
			size, strerror(errno));
		free(area);
		area = NULL;
	}
	if (result == data_directory) fclose(fp);
	/* XXX: check checksum */
	return area;
}

// seg009:A172
int __pascal far load_from_opendats_to_area(int resource,void far *area,int length, const char* extension) {
	// stub
	//return 0;
	dat_type* pointer;
	data_location result;
	byte checksum;
	int size;
	FILE* fp = NULL;
	load_from_opendats_metadata(resource, extension, &fp, &result, &checksum, &size, &pointer);
	if (result == data_none) return 0;
	if (fread(area, MIN(size, length), 1, fp) != 1) {
		fprintf(stderr, "%s: %s, resource %d, size %d, failed: %s\n",
			__func__, pointer->filename, resource,
			size, strerror(errno));
		memset(area, 0, MIN(size, length));
	}
	if (result == data_directory) fclose(fp);
	/* XXX: check checksum */
	return 0;
}

// SDL-specific implementations

void rect_to_sdlrect(const rect_type* rect, SDL_Rect* sdlrect) {
	sdlrect->x = rect->left;
	sdlrect->y = rect->top;
	sdlrect->w = rect->right - rect->left;
	sdlrect->h = rect->bottom - rect->top;
}

void __pascal far method_1_blit_rect(surface_type near *target_surface,surface_type near *source_surface,const rect_type far *target_rect, const rect_type far *source_rect,int blit) {
	SDL_Rect src_rect;
	rect_to_sdlrect(source_rect, &src_rect);
	SDL_Rect dest_rect;
	rect_to_sdlrect(target_rect, &dest_rect);

	if (blit == blitters_0_no_transp) {
		// Disable transparency.
		if (SDL_SetColorKey(source_surface, 0, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	} else {
		// Enable transparency.
		if (SDL_SetColorKey(source_surface, SDL_TRUE, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	}
	if (SDL_BlitSurface(source_surface, &src_rect, target_surface, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
}

image_type far * __pascal far method_3_blit_mono(image_type far *image,int xpos,int ypos,int blitter,byte color) {
	int w = image->w;
	int h = image->h;
	if (SDL_SetColorKey(image, SDL_TRUE, 0) != 0) {
		sdlperror("SDL_SetColorKey");
		quit(1);
	}
	SDL_Surface* colored_image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ARGB8888, 0);

	SDL_SetSurfaceBlendMode(colored_image, SDL_BLENDMODE_NONE);
	/* Causes problems with SDL 2.0.5 (see #105)
	if (SDL_SetColorKey(colored_image, SDL_TRUE, 0) != 0) {
		sdlperror("SDL_SetColorKey");
		quit(1);
	}
	*/

	if (SDL_LockSurface(colored_image) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}

	int y,x;
	rgb_type palette_color = palette[color];
	uint32_t rgb_color = SDL_MapRGB(colored_image->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2) & 0xFFFFFF;
	int stride = colored_image->pitch;
	for (y = 0; y < h; ++y) {
		uint32_t* pixel_ptr = (uint32_t*) ((byte*)colored_image->pixels + stride * y);
		for (x = 0; x < w; ++x) {
			// set RGB but leave alpha
			*pixel_ptr = (*pixel_ptr & 0xFF000000) | rgb_color;
			//printf("pixel x=%d, y=%d, color = 0x%8x\n", x, y, *pixel_ptr);
			++pixel_ptr;
		}
	}
	SDL_UnlockSurface(colored_image);

	SDL_Rect src_rect = {0, 0, image->w, image->h};
	SDL_Rect dest_rect = {xpos, ypos, image->w, image->h};

	SDL_SetSurfaceBlendMode(colored_image, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceBlendMode(current_target_surface, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(colored_image, 255);
	if (SDL_BlitSurface(colored_image, &src_rect, current_target_surface, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
	SDL_FreeSurface(colored_image);

	return image;
}

// Workaround for a bug in SDL2 (before v2.0.4):
// https://bugzilla.libsdl.org/show_bug.cgi?id=2986
// SDL_FillRect onto a 24-bit surface swaps Red and Blue component

bool RGB24_bug_checked = false;
bool RGB24_bug_affected;

bool RGB24_bug_check() {
	if (!RGB24_bug_checked) {
		// Check if the bug occurs in this version of SDL.
		SDL_Surface* test_surface = SDL_CreateRGBSurface(0, 1, 1, 24, 0, 0, 0, 0);
		if (NULL == test_surface) sdlperror("SDL_CreateSurface in RGB24_bug_check");
		// Fill with red.
		SDL_FillRect(test_surface, NULL, SDL_MapRGB(test_surface->format, 0xFF, 0, 0));
		if (0 != SDL_LockSurface(test_surface)) sdlperror("SDL_LockSurface in RGB24_bug_check");
		// Read red component of pixel.
		RGB24_bug_affected = (*(Uint32*)test_surface->pixels & test_surface->format->Rmask) == 0;
		SDL_UnlockSurface(test_surface);
		SDL_FreeSurface(test_surface);
		RGB24_bug_checked = true;
	}
	return RGB24_bug_affected;
}

int safe_SDL_FillRect(SDL_Surface* dst, const SDL_Rect* rect, Uint32 color) {
	if (dst->format->BitsPerPixel == 24 && RGB24_bug_check()) {
		// In the buggy version, SDL_FillRect swaps R and B, so we swap it once more.
		color = ((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16);
	}
	return SDL_FillRect(dst, rect, color);
}
// End of workaround.

const rect_type far * __pascal far method_5_rect(const rect_type far *rect,int blit,byte color) {
	SDL_Rect dest_rect;
	rect_to_sdlrect(rect, &dest_rect);
	rgb_type palette_color = palette[color];
#ifndef USE_ALPHA
	uint32_t rgb_color = SDL_MapRGB(onscreen_surface_->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2);
#else
	uint32_t rgb_color = SDL_MapRGBA(current_target_surface->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2, color == 0 ? SDL_ALPHA_TRANSPARENT : SDL_ALPHA_OPAQUE);
#endif
	if (safe_SDL_FillRect(current_target_surface, &dest_rect, rgb_color) != 0) {
		sdlperror("SDL_FillRect");
		quit(1);
	}
	return rect;
}

void draw_rect_with_alpha(const rect_type* rect, byte color, byte alpha) {
	SDL_Rect dest_rect;
	rect_to_sdlrect(rect, &dest_rect);
	rgb_type palette_color = palette[color];
	uint32_t rgb_color = SDL_MapRGBA(overlay_surface->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2, alpha);
	if (safe_SDL_FillRect(current_target_surface, &dest_rect, rgb_color) != 0) {
		sdlperror("SDL_FillRect");
		quit(1);
	}
}

void blit_xor(SDL_Surface* target_surface, SDL_Rect* dest_rect, SDL_Surface* image, SDL_Rect* src_rect) {
	if (dest_rect->w != src_rect->w || dest_rect->h != src_rect->h) {
		printf("blit_xor: dest_rect and src_rect have different sizes\n");
		quit(1);
	}
	SDL_Surface* helper_surface = SDL_CreateRGBSurface(0, dest_rect->w, dest_rect->h, 24, 0xFF, 0xFF<<8, 0xFF<<16, 0);
	if (helper_surface == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	SDL_Surface* image_24 = SDL_ConvertSurface(image, helper_surface->format, 0);
	//SDL_CreateRGBSurface(0, src_rect->w, src_rect->h, 24, 0xFF, 0xFF<<8, 0xFF<<16, 0);
	if (image_24 == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	SDL_Rect dest_rect2 = *src_rect;
	// Read what is currently where we want to draw the new image.
	if (SDL_BlitSurface(target_surface, dest_rect, helper_surface, &dest_rect2) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
	if (SDL_LockSurface(image_24) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	if (SDL_LockSurface(helper_surface) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	int size = helper_surface->h * helper_surface->pitch;
	int i;
	byte *p_src = (byte*) image_24->pixels;
	byte *p_dest = (byte*) helper_surface->pixels;

	// Xor the old area with the image.
	for (i = 0; i < size; ++i) {
		*p_dest ^= *p_src;
		++p_src; ++p_dest;
	}
	SDL_UnlockSurface(image_24);
	SDL_UnlockSurface(helper_surface);
	// Put the new area in place of the old one.
	if (SDL_BlitSurface(helper_surface, src_rect, target_surface, dest_rect) != 0) {
		sdlperror("SDL_BlitSurface 2065");
		quit(1);
	}
	SDL_FreeSurface(image_24);
	SDL_FreeSurface(helper_surface);
}

image_type far * __pascal far method_6_blit_img_to_scr(image_type far *image,int xpos,int ypos,int blit) {
	if (image == NULL) {
		printf("method_6_blit_img_to_scr: image == NULL\n");
		//quit(1);
		return NULL;
	}

	if (blit == blitters_9_black) {
		method_3_blit_mono(image, xpos, ypos, blitters_9_black, 0);
		return image;
	}

	SDL_Rect src_rect = {0, 0, image->w, image->h};
	SDL_Rect dest_rect = {xpos, ypos, image->w, image->h};

	if (blit == blitters_3_xor) {
		blit_xor(current_target_surface, &dest_rect, image, &src_rect);
		return image;
	}
	SDL_SetSurfaceBlendMode(image, SDL_BLENDMODE_NONE);
	SDL_SetSurfaceBlendMode(current_target_surface, SDL_BLENDMODE_NONE);
	SDL_SetSurfaceAlphaMod(image, 255);

	if (blit == blitters_0_no_transp) {
		SDL_SetColorKey(image, SDL_FALSE, 0);
	}
	else {
		SDL_SetColorKey(image, SDL_TRUE, 0);
	}
	if (SDL_BlitSurface(image, &src_rect, current_target_surface, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface 2247");
		quit(1);
	}

	if (SDL_SetSurfaceAlphaMod(image, 0) != 0) {
		sdlperror("SDL_SetAlpha");
		quit(1);
	}
	return image;
}

#ifndef USE_COMPAT_TIMER
int fps = 60;
float milliseconds_per_tick = (1000.0f / 60.0f);
Uint64 timer_last_counter[2];
#endif
int wait_time[2];


#ifdef USE_COMPAT_TIMER
Uint32 timer_callback(Uint32 interval, void *param) {
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_USEREVENT;
	event.user.code = userevent_TIMER;
	event.user.data1 = param;
	SDL_PushEvent(&event);
	return interval;
}
#endif

void reset_timer(int timer_index) {
#ifndef USE_COMPAT_TIMER
	timer_last_counter[timer_index] = SDL_GetPerformanceCounter();
#endif
}

void set_timer_length(int timer_index, int length) {
	wait_time[timer_index] = length;
}

void __pascal start_timer(int timer_index, int length) {
#ifdef USE_REPLAY
	if (replaying && skipping_replay) return;
#endif
#ifndef USE_COMPAT_TIMER
	timer_last_counter[timer_index] = SDL_GetPerformanceCounter();
#endif
	wait_time[timer_index] = length;
}

void toggle_fullscreen() {
	uint32_t flags = SDL_GetWindowFlags(window_);
	if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		SDL_SetWindowFullscreen(window_, 0);
		SDL_ShowCursor(SDL_ENABLE);
	}
	else {
		SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_ShowCursor(SDL_DISABLE);
	}
}

void process_events() {
	// Process all events in the queue.
	SDL_Event event;
	while (SDL_PollEvent(&event) == 1) { // while there are still events to be processed
		switch (event.type) {
			case SDL_KEYDOWN:
			{
				int modifier = event.key.keysym.mod;
				int scancode = event.key.keysym.scancode;

				// Handle these separately, so they won't interrupt things that are usually interrupted by a keypress. (pause, cutscene)
#ifdef USE_SCREENSHOT
				if (scancode == SDL_SCANCODE_F12) {
					if (modifier & KMOD_SHIFT) {
						save_level_screenshot((modifier & KMOD_CTRL) != 0);
					} else {
						save_screenshot();
					}
				} else
#endif
				if ((modifier & KMOD_ALT) &&
				    scancode == SDL_SCANCODE_RETURN)
				{
					// Only if the Enter key was pressed down right now.
					if (key_states[scancode] == 0) {
						// Alt-Enter: toggle fullscreen mode
						toggle_fullscreen();
						key_states[scancode] = 1;
					}
				} else {
					key_states[scancode] = 1;
					switch (scancode) {
						// Keys that are ignored by themselves:
						case SDL_SCANCODE_LCTRL:
						case SDL_SCANCODE_LSHIFT:
						case SDL_SCANCODE_LALT:
						case SDL_SCANCODE_LGUI:
						case SDL_SCANCODE_RCTRL:
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_RALT:
						case SDL_SCANCODE_RGUI:
						case SDL_SCANCODE_CAPSLOCK:
						case SDL_SCANCODE_SCROLLLOCK:
						case SDL_SCANCODE_NUMLOCKCLEAR:
						case SDL_SCANCODE_APPLICATION:
						case SDL_SCANCODE_PRINTSCREEN:
						case SDL_SCANCODE_PAUSE:
							break;
						default:
							last_key_scancode = scancode;
							if (modifier & KMOD_SHIFT) last_key_scancode |= WITH_SHIFT;
							if (modifier & KMOD_CTRL ) last_key_scancode |= WITH_CTRL ;
							if (modifier & KMOD_ALT  ) last_key_scancode |= WITH_ALT  ;
					}

#ifdef USE_AUTO_INPUT_MODE
					switch (scancode) {
						// Keys that are used for keyboard control:
						case SDL_SCANCODE_LSHIFT:
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_CLEAR:
						case SDL_SCANCODE_HOME:
						case SDL_SCANCODE_PAGEUP:
						case SDL_SCANCODE_KP_2:
						case SDL_SCANCODE_KP_4:
						case SDL_SCANCODE_KP_5:
						case SDL_SCANCODE_KP_6:
						case SDL_SCANCODE_KP_7:
						case SDL_SCANCODE_KP_8:
						case SDL_SCANCODE_KP_9:
							if (!is_keyboard_mode) {
								is_keyboard_mode = 1;
								is_joyst_mode = 0;
							}
					}
#endif
				}
				break;
			}
			case SDL_KEYUP:
				key_states[event.key.keysym.scancode] = 0;
				break;
			case SDL_CONTROLLERAXISMOTION:
				if (event.caxis.axis < 6) {
					joy_axis[event.caxis.axis] = event.caxis.value;

#ifdef USE_AUTO_INPUT_MODE
					if (!is_joyst_mode && (event.caxis.value >= JOY_THRESHOLD || event.caxis.value <= -JOY_THRESHOLD)) {
						is_joyst_mode = 1;
						is_keyboard_mode = 0;
					}
#endif
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
#ifdef USE_AUTO_INPUT_MODE
				if (!is_joyst_mode) {
					is_joyst_mode = 1;
					is_keyboard_mode = 0;
				}
#endif
				switch (event.cbutton.button)
				{
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  joy_hat_states[0] = -1; break; // left
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: joy_hat_states[0] = 1;  break; // right
					case SDL_CONTROLLER_BUTTON_DPAD_UP:    joy_hat_states[1] = -1; break; // up
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  joy_hat_states[1] = 1;  break; // down

					case SDL_CONTROLLER_BUTTON_A:          joy_AY_buttons_state = 1;  break; /*** A (down) ***/
					case SDL_CONTROLLER_BUTTON_Y:          joy_AY_buttons_state = -1; break; /*** Y (up) ***/
					case SDL_CONTROLLER_BUTTON_X:          joy_X_button_state = 1;    break; /*** X (shift) ***/
					case SDL_CONTROLLER_BUTTON_B:          joy_B_button_state = 1;    break; /*** B (unused) ***/

					case SDL_CONTROLLER_BUTTON_START:
						last_key_scancode = SDL_SCANCODE_R | WITH_CTRL; /*** start (restart game) ***/
						break;

					case SDL_CONTROLLER_BUTTON_BACK:
						last_key_scancode = SDL_SCANCODE_A | WITH_CTRL;  /*** back (restart level) ***/
						break;

					default: break;
				}
				break;
			case SDL_CONTROLLERBUTTONUP:
				switch (event.cbutton.button)
				{
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  joy_hat_states[0] = 0; break; // left
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: joy_hat_states[0] = 0; break; // right
					case SDL_CONTROLLER_BUTTON_DPAD_UP:    joy_hat_states[1] = 0; break; // up
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  joy_hat_states[1] = 0; break; // down

					case SDL_CONTROLLER_BUTTON_A:          joy_AY_buttons_state = 0; break; /*** A (down) ***/
					case SDL_CONTROLLER_BUTTON_Y:          joy_AY_buttons_state = 0; break; /*** Y (up) ***/
					case SDL_CONTROLLER_BUTTON_X:          joy_X_button_state = 0;   break; /*** X (shift) ***/
					case SDL_CONTROLLER_BUTTON_B:          joy_B_button_state = 0;   break; /*** B (unused) ***/

					default: break;
				}
				break;
			case SDL_JOYBUTTONDOWN:
#ifdef USE_AUTO_INPUT_MODE
				if (!is_joyst_mode) {
					is_joyst_mode = 1;
					is_keyboard_mode = 0;
				}
#endif
				switch (event.jbutton.button)
				{
					case SDL_JOYSTICK_BUTTON_Y:            joy_AY_buttons_state = -1; break; /*** Y (up) ***/
					case SDL_JOYSTICK_BUTTON_X:            joy_X_button_state = 1;    break; /*** X (shift) ***/
				}
				break;
			case SDL_JOYBUTTONUP:
				switch (event.jbutton.button)
				{
					case SDL_JOYSTICK_BUTTON_Y:            joy_AY_buttons_state = 0; break; /*** Y (up) ***/
					case SDL_JOYSTICK_BUTTON_X:            joy_X_button_state = 0;   break; /*** X (shift) ***/
						break;

				}
				break;
			case SDL_JOYAXISMOTION:
#ifdef USE_AUTO_INPUT_MODE
				if (!is_joyst_mode) {
					is_joyst_mode = 1;
					is_keyboard_mode = 0;
				}
#endif
				switch (event.jaxis.axis)
				{
					case SDL_JOYSTICK_X_AXIS:
						if (event.jaxis.value < 0)  joy_hat_states[0] = -1; // left
						if (event.jaxis.value > 0)  joy_hat_states[0] = 1;  // right
						if (event.jaxis.value == 0) joy_hat_states[0] = 0;  // axis released
						break;

					case SDL_JOYSTICK_Y_AXIS:
						if (event.jaxis.value < 0)  joy_hat_states[1] = -1; // up
						if (event.jaxis.value > 0)  joy_hat_states[1] = 1;  // down
						if (event.jaxis.value == 0) joy_hat_states[1] = 0;  // axis released
						break;
				}
				break;
			case SDL_TEXTINPUT:
				last_text_input = event.text.text[0]; // UTF-8 formatted char text input
				break;
			case SDL_WINDOWEVENT:
				// In case the user switches away while holding a key: do as if all keys were released.
				// (DOSBox does the same.)

/* // not implemented in SDL2 for now
 *
			if ((event.active.state & SDL_APPINPUTFOCUS) && event.active.gain == 0) {
				memset(key_states, 0, sizeof(key_states));
			}
			// Note: event.active.state can contain multiple flags or'ed.
			// If the game is in full screen, and I switch away (alt-tab) and back, most of the screen will be black, until it is redrawn.
			if ((event.active.state & SDL_APPACTIVE) && event.active.gain == 1) {
				update_screen();
			}
*/
				switch (event.window.event) {
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						//case SDL_WINDOWEVENT_MOVED:
						//case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_EXPOSED:
						update_screen();
						break;
				}
				break;
			case SDL_USEREVENT:
				if (event.user.code == userevent_TIMER /*&& event.user.data1 == (void*)timer_index*/) {
#ifdef USE_COMPAT_TIMER
					int index;
					for (index = 0; index < 2; ++index) {
						if (wait_time[index] > 0) --wait_time[index];
					}
#endif
				} else if (event.user.code == userevent_SOUND) {
					//sound_timer = 0;
#ifndef USE_MIXER
					//stop_sounds();
#endif
				}
				break;
			case SDL_QUIT:
				quit(0);
				break;
		}
	}
}

void idle() {
	// Previously, this procedure would wait for *one* event and process it, then return.
	// Much like the x86 HLT instruction.
	// (Although we still wanted to process all events in the queue. For instance, there might be
	// simultaneous SDL2 KEYDOWN and TEXTINPUT events.)

	// This also meant that the main thread would stall at idle() for an unpredictable amount of time.
	// Screen updates had to be timed using SDL_Timer events, and it was not really possible to exactly match
	// the monitor's refresh rate.

	// Now, every time idle() is called, the screen is also updated.
	// * After all SDL events have been processed, we can rely on VSync to provide a suitable delay in execution.
	// * Basically, all places in the code where idle() is called are also appropriate places to update the screen.
	// * VSync acts like a 'pacemaker': we can use it to get a stable framerate. See: has_timer_stopped()
	// * The game's update frequency (either 12 or 10 frames per second) is now decoupled from screen updates.
	//   So it is also possible to overlay other graphical elements that update at a faster rate
	//   (e.g. the monitor's refresh rate).

	process_events();
	update_screen();
}
word word_1D63A = 1;
int __pascal do_wait(int timer_index) {
	while (! has_timer_stopped(timer_index)) {
		idle();
		int key = do_paused();
		if (key != 0 && (word_1D63A != 0 || key == 0x1B)) return 1;
	}
	return 0;
}

void __pascal do_simple_wait(int timer_index) {
#ifdef USE_REPLAY
	if ((replaying && skipping_replay) || is_validate_mode) return;
#endif

	while (! has_timer_stopped(timer_index)) {
		idle();
	}
}

#ifdef USE_COMPAT_TIMER
SDL_TimerID global_timer = NULL;
#endif
// seg009:78E9
void __pascal far init_timer(int frequency) {
#ifndef USE_COMPAT_TIMER
	fps = frequency;
	milliseconds_per_tick = 1000.0f / (float)fps;
#else
	if (global_timer != 0) {
		if (!SDL_RemoveTimer(global_timer)) {
			sdlperror("SDL_RemoveTimer");
		}
	}
	global_timer = SDL_AddTimer(1000/frequency, timer_callback, NULL);
	if (global_timer == 0) {
		sdlperror("SDL_AddTimer");
		quit(1);
	}
#endif
}

// seg009:35F6
void __pascal far set_clip_rect(const rect_type far *rect) {
	SDL_Rect clip_rect;
	rect_to_sdlrect(rect, &clip_rect);
	SDL_SetClipRect(current_target_surface, &clip_rect);
}

// seg009:365C
void __pascal far reset_clip_rect() {
	SDL_SetClipRect(current_target_surface, NULL);
}

// seg009:1983
void __pascal far set_bg_attr(int vga_pal_index,int hc_pal_index) {
	// stub
#ifdef USE_FLASH
	//palette[vga_pal_index] = vga_palette[hc_pal_index];
	if (!enable_flash) return;
	if (vga_pal_index == 0) {
		/*
		if (SDL_SetAlpha(offscreen_surface, SDL_SRCALPHA, 0) != 0) {
			sdlperror("SDL_SetAlpha");
			quit(1);
		}
		*/
		// Make the black pixels transparent.
		if (SDL_SetColorKey(offscreen_surface, SDL_TRUE, 0) != 0) {	// SDL_SRCCOLORKEY old
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
		SDL_Rect rect = {0,0,0,0};
		rect.w = offscreen_surface->w;
		rect.h = offscreen_surface->h;
		rgb_type palette_color = palette[hc_pal_index];
		uint32_t rgb_color = SDL_MapRGB(onscreen_surface_->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2) /*& 0xFFFFFF*/;
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0);
		// First clear the screen with the color of the flash.
		if (safe_SDL_FillRect(onscreen_surface_, &rect, rgb_color) != 0) {
			sdlperror("SDL_FillRect");
			quit(1);
		}
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0);
		if (upside_down) {
			flip_screen(offscreen_surface);
		}
		// Then draw the offscreen image onto it.
		if (SDL_BlitSurface(offscreen_surface, &rect, onscreen_surface_, &rect) != 0) {
			sdlperror("SDL_BlitSurface");
			quit(1);
		}
#ifdef USE_LIGHTING
		if (hc_pal_index == 0) update_lighting(&rect_top);
#endif
		if (upside_down) {
			flip_screen(offscreen_surface);
		}
		// And show it!
		update_screen();
		// Give some time to show the flash.
		//SDL_Flip(onscreen_surface_);
//		if (hc_pal_index != 0) SDL_Delay(2*(1000/60));
		//SDL_Flip(onscreen_surface_);
		/*
		if (SDL_SetAlpha(offscreen_surface, 0, 0) != 0) {
			sdlperror("SDL_SetAlpha");
			quit(1);
		}
		*/
		if (SDL_SetColorKey(offscreen_surface, 0, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	}
#endif // USE_FLASH
}

// seg009:07EB
rect_type far *__pascal offset4_rect_add(rect_type far *dest,const rect_type far *source,int d_left,int d_top,int d_right,int d_bottom) {
	*dest = *source;
	dest->left += d_left;
	dest->top += d_top;
	dest->right += d_right;
	dest->bottom += d_bottom;
	return dest;
}

// seg009:3AA5
rect_type far *__pascal offset2_rect(rect_type far *dest,const rect_type far *source,int delta_x,int delta_y) {
	dest->top    = source->top    + delta_y;
	dest->left   = source->left   + delta_x;
	dest->bottom = source->bottom + delta_y;
	dest->right  = source->right  + delta_x;
	return dest;
}

#ifdef USE_FADE
// seg009:19EF
void __pascal far fade_in_2(surface_type near *source_surface,int which_rows) {
	palette_fade_type far* palette_buffer;
	if (graphics_mode == gmMcgaVga) {
		palette_buffer = make_pal_buffer_fadein(source_surface, which_rows, 2);
		while (fade_in_frame(palette_buffer) == 0) {
			process_events(); // modified
			do_paused();
		}
		pal_restore_free_fadein(palette_buffer);
	} else {
		// ...
	}
}

// seg009:1A51
palette_fade_type far *__pascal make_pal_buffer_fadein(surface_type *source_surface,int which_rows,int wait_time) {
	palette_fade_type far* palette_buffer;
	word curr_row;
	word var_8;
	word curr_row_mask;
	palette_buffer = (palette_fade_type*) malloc_far(sizeof(palette_fade_type));
	palette_buffer->which_rows = which_rows;
	palette_buffer->wait_time = wait_time;
	palette_buffer->fade_pos = 0x40;
	palette_buffer->proc_restore_free = &pal_restore_free_fadein;
	palette_buffer->proc_fade_frame = &fade_in_frame;
	read_palette_256(palette_buffer->original_pal);
	memcpy_far(palette_buffer->faded_pal, palette_buffer->original_pal, sizeof(palette_buffer->faded_pal));
	var_8 = 0;
	for (curr_row = 0, curr_row_mask = 1; curr_row < 0x10; ++curr_row, curr_row_mask<<=1) {
		if (which_rows & curr_row_mask) {
			memset_far(palette_buffer->faded_pal + (curr_row<<4), 0, sizeof(rgb_type[0x10]));
			set_pal_arr(curr_row<<4, 0x10, NULL, (var_8++&3)==0);
		}
	}
	//method_1_blit_rect(onscreen_surface_, source_surface, &screen_rect, &screen_rect, 0);
	// for RGB
	//method_5_rect(&screen_rect, 0, 0);
	return palette_buffer;
}

// seg009:1B64
void __pascal far pal_restore_free_fadein(palette_fade_type far *palette_buffer) {
	set_pal_256(palette_buffer->original_pal);
	free_far(palette_buffer);
	// for RGB
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, 0);
}

// seg009:1B88
int __pascal far fade_in_frame(palette_fade_type far *palette_buffer) {
	rgb_type* faded_pal_ptr;
	word start;
	word column;
	rgb_type* original_pal_ptr;
	word current_row_mask;
//	void* var_12;
	/**/start_timer(timer_1, palette_buffer->wait_time); // too slow?

	//printf("start ticks = %u\n",SDL_GetTicks());
	--palette_buffer->fade_pos;
	for (start=0,current_row_mask=1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			//var_12 = palette_buffer->
			original_pal_ptr = palette_buffer->original_pal + start;
			faded_pal_ptr = palette_buffer->faded_pal + start;
			for (column = 0; column<0x10; ++column) {
				if (original_pal_ptr[column].r > palette_buffer->fade_pos) {
					++faded_pal_ptr[column].r;
				}
				if (original_pal_ptr[column].g > palette_buffer->fade_pos) {
					++faded_pal_ptr[column].g;
				}
				if (original_pal_ptr[column].b > palette_buffer->fade_pos) {
					++faded_pal_ptr[column].b;
				}
			}
		}
	}
	column = 0;
	for (start = 0, current_row_mask = 1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			set_pal_arr(start, 0x10, palette_buffer->faded_pal + start, (column++&3)==0);
		}
	}

	int h = offscreen_surface->h;
	if (SDL_LockSurface(onscreen_surface_) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	if (SDL_LockSurface(offscreen_surface) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	int y,x;
	int on_stride = onscreen_surface_->pitch;
	int off_stride = offscreen_surface->pitch;
	int fade_pos = palette_buffer->fade_pos;
	for (y = 0; y < h; ++y) {
		byte* on_pixel_ptr = (byte*)onscreen_surface_->pixels + on_stride * y;
		byte* off_pixel_ptr = (byte*)offscreen_surface->pixels + off_stride * y;
		for (x = 0; x < on_stride; ++x) {
			//if (*off_pixel_ptr > palette_buffer->fade_pos) *pixel_ptr += 4;
			int v = *off_pixel_ptr - fade_pos*4;
			if (v<0) v=0;
			*on_pixel_ptr = v;
			++on_pixel_ptr; ++off_pixel_ptr;
		}
	}
	SDL_UnlockSurface(onscreen_surface_);
	SDL_UnlockSurface(offscreen_surface);

	//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug

	/**/do_simple_wait(1); // can interrupt fading of cutscene
	//do_wait(timer_1); // can interrupt fading of main title
	//printf("end ticks = %u\n",SDL_GetTicks());
	return palette_buffer->fade_pos == 0;
}

// seg009:1CC9
void __pascal far fade_out_2(int rows) {
	palette_fade_type far *palette_buffer;
	if (graphics_mode == gmMcgaVga) {
		palette_buffer = make_pal_buffer_fadeout(rows, 2);
		while (fade_out_frame(palette_buffer) == 0) {
			process_events(); // modified
			do_paused();
		}
		pal_restore_free_fadeout(palette_buffer);
	} else {
		// ...
	}
}

// seg009:1D28
palette_fade_type far *__pascal make_pal_buffer_fadeout(int which_rows,int wait_time) {
	palette_fade_type far *palette_buffer;
	palette_buffer = (palette_fade_type*) malloc_far(sizeof(palette_fade_type));
	palette_buffer->which_rows = which_rows;
	palette_buffer->wait_time = wait_time;
	palette_buffer->fade_pos = 00; // modified
	palette_buffer->proc_restore_free = &pal_restore_free_fadeout;
	palette_buffer->proc_fade_frame = &fade_out_frame;
	read_palette_256(palette_buffer->original_pal);
	memcpy_far(palette_buffer->faded_pal, palette_buffer->original_pal, sizeof(palette_buffer->faded_pal));
	// for RGB
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, 0);
	return palette_buffer;
}

// seg009:1DAF
void __pascal far pal_restore_free_fadeout(palette_fade_type far *palette_buffer) {
	surface_type* surface;
	surface = current_target_surface;
	current_target_surface = onscreen_surface_;
	draw_rect(&screen_rect, 0);
	current_target_surface = surface;
	set_pal_256(palette_buffer->original_pal);
	free_far(palette_buffer);
	// for RGB
	method_5_rect(&screen_rect, 0, 0);
}

// seg009:1DF7
int __pascal far fade_out_frame(palette_fade_type far *palette_buffer) {
	rgb_type* faded_pal_ptr;
	word start;
	word var_8;
	word column;
	word current_row_mask;
	byte* curr_color_ptr;
	var_8 = 1;
	++palette_buffer->fade_pos; // modified
	/**/start_timer(timer_1, palette_buffer->wait_time); // too slow?
	for (start=0,current_row_mask=1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			//var_12 = palette_buffer->
			//original_pal_ptr = palette_buffer->original_pal + start;
			faded_pal_ptr = palette_buffer->faded_pal + start;
			for (column = 0; column<0x10; ++column) {
				curr_color_ptr = &faded_pal_ptr[column].r;
				if (*curr_color_ptr != 0) {
					--*curr_color_ptr;
					var_8 = 0;
				}
				curr_color_ptr = &faded_pal_ptr[column].g;
				if (*curr_color_ptr != 0) {
					--*curr_color_ptr;
					var_8 = 0;
				}
				curr_color_ptr = &faded_pal_ptr[column].b;
				if (*curr_color_ptr != 0) {
					--*curr_color_ptr;
					var_8 = 0;
				}
			}
		}
	}
	column = 0;
	for (start = 0, current_row_mask = 1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			set_pal_arr(start, 0x10, palette_buffer->faded_pal + start, (column++&3)==0);
		}
	}

	int h = offscreen_surface->h;
	if (SDL_LockSurface(onscreen_surface_) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	if (SDL_LockSurface(offscreen_surface) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	int y,x;
	int on_stride = onscreen_surface_->pitch;
	int off_stride = offscreen_surface->pitch;
	int fade_pos = palette_buffer->fade_pos;
	for (y = 0; y < h; ++y) {
		byte* on_pixel_ptr = (byte*)onscreen_surface_->pixels + on_stride * y;
		byte* off_pixel_ptr = (byte*)offscreen_surface->pixels + off_stride * y;
		for (x = 0; x < on_stride; ++x) {
			//if (*pixel_ptr >= 4) *pixel_ptr -= 4;
			int v = *off_pixel_ptr - fade_pos*4;
			if (v<0) v=0;
			*on_pixel_ptr = v;
			++on_pixel_ptr; ++off_pixel_ptr;
		}
	}
	SDL_UnlockSurface(onscreen_surface_);
	SDL_UnlockSurface(offscreen_surface);

	do_simple_wait(timer_1); // can interrupt fading of cutscene
	return var_8;
}

// seg009:1F28
void __pascal far read_palette_256(rgb_type far *target) {
	int i;
	for (i = 0; i < 256; ++i) {
		target[i] = palette[i];
	}
}

// seg009:1F5E
void __pascal far set_pal_256(rgb_type far *source) {
	int i;
	for (i = 0; i < 256; ++i) {
		palette[i] = source[i];
	}
}
#endif // USE_FADE

void set_chtab_palette(chtab_type* chtab, byte* colors, int n_colors) {
	if (chtab != NULL) {
		SDL_Color* scolors = (SDL_Color*) malloc(n_colors*sizeof(SDL_Color));
		int i;
		//printf("scolors\n",i);
		for (i = 0; i < n_colors; ++i) {
			//printf("i=%d\n",i);
			scolors[i].r = *colors << 2; ++colors;
			scolors[i].g = *colors << 2; ++colors;
			scolors[i].b = *colors << 2; ++colors;
			scolors[i].a = SDL_ALPHA_OPAQUE; // the SDL2 SDL_Color struct has an alpha component
		}

		// Color 0 of the palette data is not used, it is replaced by the background color.
		// Needed for correct alternate colors (v1.3) of level 8.
		scolors[0].r = scolors[0].g = scolors[0].b = 0;

		//printf("setcolors\n",i);
		for (i = 0; i < chtab->n_images; ++i) {
			//printf("i=%d\n",i);
			image_type* current_image = chtab->images[i];
			if (current_image != NULL) {

				int n_colors_to_be_set = n_colors;
				SDL_Palette* current_palette = current_image->format->palette;

				// one of the guard images (i=25) is only a single transparent pixel
				// this caused SDL_SetPaletteColors to fail, I think because that palette contains only 2 colors
				if (current_palette->ncolors < n_colors_to_be_set)
					n_colors_to_be_set = current_palette->ncolors;
				if (SDL_SetPaletteColors(current_palette, scolors, 0, n_colors_to_be_set) != 0) {
					sdlperror("SDL_SetPaletteColors");
					quit(1);
				}
			}
		}
		free(scolors);
	}
}

int has_timer_stopped(int timer_index) {
#ifdef USE_COMPAT_TIMER
	return wait_time[timer_index] == 0;
#else
#ifdef USE_REPLAY
	if ((replaying && skipping_replay) || is_validate_mode) return true;
#endif
	float milliseconds_for_cpu_overhead = 0.0f;
	if (vsync_enabled) {
		// Before the *next* refresh occurs, we want to reserve CPU time so we can process the next frame in advance.
		// Ideally, this would be as low as possible (reduce input latency!)
		// On the other hand, we don't want to drop frames because we started too late on preparing the next one.
		milliseconds_for_cpu_overhead = 12.0f;

		// The CPU time for the next frame must be shorter than the refresh time itself.
		// This can become a problem on fast monitors.
		// For example, for a 240Hz display, the time for cpu processing can at most be 1000.0f / 240.0f = 4.1667 ms.
		float refresh_time = get_display_refresh_time();
		float max_cpu_time = 0.75f * refresh_time;
		if (milliseconds_for_cpu_overhead > max_cpu_time) {
			milliseconds_for_cpu_overhead = max_cpu_time;
		}
	}

	float timer_length_milliseconds = wait_time[timer_index] * milliseconds_per_tick;
	Uint64 current_counter = SDL_GetPerformanceCounter();
	float milliseconds_elapsed = (current_counter - timer_last_counter[timer_index]) * milliseconds_per_counter;
	float milliseconds_left = timer_length_milliseconds - milliseconds_elapsed;
	if (milliseconds_left < milliseconds_for_cpu_overhead) {
		timer_last_counter[timer_index] = current_counter;
//		printf("timer %d:   frametime (ms) = %5.1f    fps = %.1f\n", timer_index, milliseconds_elapsed, 1000.0f / milliseconds_elapsed);
		return true;
	} else {
		return false;
	}
#endif
}
