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

#ifndef _MSC_VER // unistd.h does not exist in the Windows SDK.
#include <unistd.h>
#endif
#include <fcntl.h>

// data:4CB4
short cutscene_wait_frames;
// data:3D14
short cutscene_frame_time;
// data:588C
short disable_keys;
// data:436A
short hourglass_sandflow;
// data:5964
short hourglass_state;
// data:4CC4
short which_torch;

#pragma pack(push,1)
typedef struct hof_type {
	char name[25];
	short min,tick;
} hof_type;
SDL_COMPILE_TIME_ASSERT(hof_size, sizeof(hof_type) == 29);
#pragma pack(pop)

#define MAX_HOF_COUNT 6
// data:405E
short hof_count;
// data:589A
hof_type hof[MAX_HOF_COUNT];

#define N_STARS 6

// data:0D92
rect_type hof_rects[MAX_HOF_COUNT] = {
{ 84,   72,   96,  248},
{ 98,   72,  110,  248},
{112,   72,  124,  248},
{126,   72,  138,  248},
{140,   72,  152,  248},
{154,   72,  166,  248},
};

// seg001:0004
int __pascal far proc_cutscene_frame(int wait_frames) {
	cutscene_wait_frames = wait_frames;
	do {
		start_timer(timer_0, cutscene_frame_time);
		play_both_seq();
		draw_proom_drects(); // changed order of drects and flash
		if (flash_time) {
			do_flash(flash_color);
		}
		if (flash_time) {
			--flash_time;
			remove_flash();
		}
		if (!check_sound_playing()) {
			play_next_sound();
		}
		do {
			if (!disable_keys && do_paused()) {
				stop_sounds();
				draw_rect(&screen_rect, 0);
#ifdef USE_FADE
				if (is_global_fading) {
					fade_palette_buffer->proc_restore_free(fade_palette_buffer);
					is_global_fading = 0;
				}
#endif
				return 1;
			}
#ifdef USE_FADE
			if (is_global_fading) {
				if (fade_palette_buffer->proc_fade_frame(fade_palette_buffer)) {
					fade_palette_buffer->proc_restore_free(fade_palette_buffer);
					is_global_fading = 0;
					return 2;
				}
			} else {
				idle();
			}
#else
			idle();
#endif
		} while(!has_timer_stopped(0)); // busy waiting?
	} while(--cutscene_wait_frames);
	return 0;
}

// seg001:00DD
void __pascal far play_both_seq() {
	play_kid_seq();
	play_opp_seq();
}

// seg001:00E6
void __pascal far draw_proom_drects() {
	draw_princess_room_bg();
#ifdef USE_FADE
	if (!is_global_fading) {
#endif
	while (drects_count--) {
		copy_screen_rect(&drects[drects_count]);
	}
#ifdef USE_FADE
	}
#endif
	drects_count = 0;
	if (cutscene_wait_frames & 1) {
		draw_star(prandom(N_STARS - 1), 1);
	}
}

// seg001:0128
void __pascal far play_kid_seq() {
	loadkid();
	if (Char.frame) {
		play_seq();
		savekid();
	}
}

// seg001:013F
void __pascal far play_opp_seq() {
	loadshad_and_opp();
	if (Char.frame) {
		play_seq();
		saveshad();
	}
}

// seg001:0156
void __pascal far draw_princess_room_bg() {
	memset_near(table_counts, 0, sizeof(table_counts));
	loadkid();
	if (Char.frame) {
		load_frame_to_obj();
		obj_tilepos = 30;
		add_objtable(0);
	}
	loadshad();
	if (Char.frame) {
		load_frame_to_obj();
		obj_tilepos = 30;
		add_objtable(0);
	}
	redraw_needed_tiles();
	add_foretable(id_chtab_8_princessroom, 2 /*pillar piece*/, 30, 0, 167, blitters_10h_transp, 0);
	princess_room_torch();
	draw_hourglass();
	draw_tables();
}

// seg001:01E0
void __pascal far seqtbl_offset_shad_char(int seq_index) {
	loadshad();
	seqtbl_offset_char(seq_index);
	saveshad();
}

// seg001:01F9
void __pascal far seqtbl_offset_kid_char(int seq_index) {
	loadkid();
	seqtbl_offset_char(seq_index);
	savekid();
}

// seg001:0212
void __pascal far init_mouse_cu8() {
	init_mouse_go();
	Char.x = 144;
	seqtbl_offset_char(seq_106_mouse); // mouse
	play_seq();
}

// seg001:022A
void __pascal far init_mouse_go() {
	Char.charid = charid_24_mouse;
	Char.x = 199;
	Char.y = 167;
	Char.direction = dir_FF_left;
	seqtbl_offset_char(seq_105_mouse_forward); // mouse go
	play_seq();
}

// seg001:024D
void __pascal far princess_crouching() {
	init_princess();
	Char.x = 131;
	Char.y = 169;
	seqtbl_offset_char(seq_110_princess_crouching_PV2); // princess crouching [PV2]
	play_seq();
}

// seg001:026A
void __pascal far princess_stand() {
	init_princess_right();
	Char.x = 144;
	Char.y = 169;
	seqtbl_offset_char(seq_94_princess_stand_PV1); // princess stand [PV1]
	play_seq();
}

// seg001:0287
void __pascal far init_princess_x156() {
	init_princess();
	Char.x = 156;
}

// seg001:0291
void __pascal far princess_lying() {
	init_princess();
	Char.x = 92;
	Char.y = 162;
	seqtbl_offset_char(seq_103_princess_lying_PV2); // princess lying [PV2]
	play_seq();
}

// seg001:02AE
void __pascal far init_princess_right() {
	init_princess();
	Char.direction = dir_0_right;
}

// seg001:02B8
void __pascal far init_ending_princess() {
	init_princess();
	Char.x = 136;
	Char.y = 164;
	seqtbl_offset_char(seq_109_princess_stand_PV2); // princess standing [PV2]
	play_seq();
}

// seg001:02D5
void __pascal far init_mouse_1() {
	init_mouse_go();
	Char.x -= 2;
	Char.y = 164;
}

// seg001:02E4
void __pascal far init_princess() {
	Char.charid = charid_5_princess;
	Char.x = 120;
	Char.y = 166;
	Char.direction = dir_FF_left;
	seqtbl_offset_char(seq_94_princess_stand_PV1); // princess stand [PV1]
	play_seq();
}

// seg001:0307
void __pascal far init_vizier() {
	Char.charid = charid_6_vizier;
	Char.x = 198;
	Char.y = 166;
	Char.direction = dir_FF_left;
	seqtbl_offset_char(seq_95_Jaffar_stand_PV1); // Jaffar stand [PV1]
	play_seq();
}

// seg001:032A
void __pascal far init_ending_kid() {
	Char.charid = charid_0_kid;
	Char.x = 198;
	Char.y = 164;
	Char.direction = dir_FF_left;
	seqtbl_offset_char(seq_1_start_run); // start run
	play_seq();
}

// seg001:034D
void __pascal far cutscene_8() {
	play_sound(sound_35_cutscene_8_9); // cutscene 8, 9
	set_hourglass_state(hourglass_frame());
	init_mouse_cu8();
	savekid();
	princess_crouching();
	saveshad();
	if (fade_in_1()) return;
	if (proc_cutscene_frame(20)) return;
	seqtbl_offset_kid_char(seq_107_mouse_stand_up_and_go); // mouse stand up and go
	if (proc_cutscene_frame(20)) return;
	seqtbl_offset_shad_char(seq_111_princess_stand_up_PV2); // princess stand up [PV2]
	if (proc_cutscene_frame(20)) return;
	Kid.frame = 0;
	fade_out_1();
}

// seg001:03B7
void __pascal far cutscene_9() {
	play_sound(sound_35_cutscene_8_9); // cutscene 8, 9
	set_hourglass_state(hourglass_frame());
	princess_stand();
	saveshad();
	if (fade_in_1()) return;
	init_mouse_go();
	savekid();
	if (proc_cutscene_frame(5)) return;
	seqtbl_offset_shad_char(seq_112_princess_crouch_down_PV2); // princess crouch down [PV2]
	if (proc_cutscene_frame(9)) return;
	seqtbl_offset_kid_char(seq_114_mouse_stand); // mouse stand
	if (proc_cutscene_frame(58)) return;
	fade_out_1();
}

// seg001:041C
void __pascal far end_sequence_anim() {
	disable_keys = 1;
	if (!is_sound_on) {
		turn_sound_on_off(0x0F);
	}
	copy_screen_rect(&screen_rect);
	play_sound(sound_26_embrace); // arrived to princess
	init_ending_princess();
	saveshad();
	init_ending_kid();
	savekid();
	if (proc_cutscene_frame(8)) return;
	seqtbl_offset_shad_char(seq_108_princess_turn_and_hug); // princess turn and hug [PV2]
	if (proc_cutscene_frame(5)) return;
	seqtbl_offset_kid_char(seq_13_stop_run); // stop run
	if (proc_cutscene_frame(2)) return;
	Kid.frame = 0;
	if (proc_cutscene_frame(39)) return;
	init_mouse_1();
	savekid();
	if (proc_cutscene_frame(9)) return;
	seqtbl_offset_kid_char(seq_101_mouse_stands_up); // mouse stands up
	if (proc_cutscene_frame(41)) return;
	fade_out_1();
	while (check_sound_playing()) idle();
}

// seg001:04D3
void __pascal far time_expired() {
	disable_keys = 1;
	set_hourglass_state(7);
	hourglass_sandflow = -1;
	play_sound(sound_36_out_of_time); // time over
	if (fade_in_1()) return;
	if (proc_cutscene_frame(2)) return;
	if (proc_cutscene_frame(100)) return;
	fade_out_1();
	while (check_sound_playing()) {
		idle();
		do_paused();
	}
}

// seg001:0525
void __pascal far cutscene_12() {
	short var_2;
	var_2 = hourglass_frame();
	if (var_2 >= 6) {
		set_hourglass_state(var_2);
		init_princess_x156();
		saveshad();
		play_sound(sound_40_cutscene_12_short_time); // cutscene 12 short time
		if (fade_in_1()) return;
		if (proc_cutscene_frame(2)) return;
		seqtbl_offset_shad_char(98); // princess turn around [PV1]
		if (proc_cutscene_frame(24)) return;
		fade_out_1();
	} else {
		cutscene_2_6();
	}
}

// seg001:0584
void __pascal far cutscene_4() {
	play_sound(sound_27_cutscene_2_4_6_12); // cutscene 2, 4, 6, 12
	set_hourglass_state(hourglass_frame());
	princess_lying();
	saveshad();
	if (fade_in_1()) return;
	if (proc_cutscene_frame(26)) return;
	fade_out_1();
}

// seg001:05B8
void __pascal far cutscene_2_6() {
	play_sound(sound_27_cutscene_2_4_6_12); // cutscene 2, 4, 6, 12
	set_hourglass_state(hourglass_frame());
	init_princess_right();
	saveshad();
	if (fade_in_1()) return;
	if (proc_cutscene_frame(26)) return;
	fade_out_1();
}

// seg001:05EC
void __pascal far pv_scene() {
	init_princess();
	saveshad();
	if (fade_in_1()) return;
	init_vizier();
	savekid();
	if (proc_cutscene_frame(2)) return;
	play_sound(sound_50_story_2_princess); // story 2: princess waiting
	do {
		if (proc_cutscene_frame(1)) return;
		//idle();
	} while(check_sound_playing());
	cutscene_frame_time = 8;
	if (proc_cutscene_frame(5)) return;
	play_sound(sound_4_gate_closing); // gate closing
	do {
		if (proc_cutscene_frame(1)) return;
	} while(check_sound_playing());
	play_sound(sound_51_princess_door_opening); // princess door opening
	if (proc_cutscene_frame(3)) return;
	seqtbl_offset_shad_char(98); // princess turn around [PV1]
	if (proc_cutscene_frame(5)) return;
	seqtbl_offset_kid_char(96); // Jaffar walk [PV1]
	if (proc_cutscene_frame(6)) return;
	play_sound(sound_53_story_3_Jaffar_comes); // story 3: Jaffar comes
	seqtbl_offset_kid_char(97); // Jaffar stop [PV1]
	if (proc_cutscene_frame(4)) return;
	if (proc_cutscene_frame(18)) return;
	seqtbl_offset_kid_char(96); // Jaffar walk [PV1]
	if (proc_cutscene_frame(30)) return;
	seqtbl_offset_kid_char(97); // Jaffar stop [PV1]
	if (proc_cutscene_frame(35)) return;
	seqtbl_offset_kid_char(102); // Jaffar conjuring [PV1]
	cutscene_frame_time = 7;
	if (proc_cutscene_frame(1)) return;
	seqtbl_offset_shad_char(99); // princess step back [PV1]
	if (proc_cutscene_frame(17)) return;
	hourglass_state = 1;
	flash_time = 5;
	flash_color = 15; // white
	do {
		if (proc_cutscene_frame(1)) return;
		//idle();
	} while(check_sound_playing());
	seqtbl_offset_kid_char(100); // Jaffar end conjuring and walk [PV1]
	hourglass_sandflow = 0;
	if (proc_cutscene_frame(6)) return;
	play_sound(sound_52_story_4_Jaffar_leaves); // story 4: Jaffar leaves
	if (proc_cutscene_frame(24)) return;
	hourglass_state = 2;
	if (proc_cutscene_frame(9)) return;
	seqtbl_offset_shad_char(113); // princess look down [PV1]
	if (proc_cutscene_frame(28)) return;
	fade_out_1();
}

// seg001:07C7
void __pascal far set_hourglass_state(int state) {
	hourglass_sandflow = 0;
	hourglass_state = state;
}

// data:0DEC
short time_bound[] = {6, 17, 33, 65};

// seg001:07DA
int __pascal far hourglass_frame() {
	short bound_index;
	for (bound_index = 0; bound_index < 4; ++bound_index) {
		if (time_bound[bound_index] > rem_min) {
			break;
		}
	}
	return 6 - bound_index;
}

// data:0DF4
short princess_torch_pos_xh[] = {11, 26};
// data:0DF8
short princess_torch_pos_xl[] = {5, 3};
// data:0DFC
short princess_torch_frame[] = {1, 6};

// seg001:0808
void __pascal far princess_room_torch() {
	short which;
	for (which = 2; which--; ) {
		which_torch = !which_torch;
		princess_torch_frame[which_torch] = get_torch_frame(princess_torch_frame[which_torch]);
		add_backtable(id_chtab_1_flameswordpotion, princess_torch_frame[which_torch] + 1, princess_torch_pos_xh[which_torch], princess_torch_pos_xl[which_torch], 116, 0, 0);
	}
}

// seg001:0863
void __pascal far draw_hourglass() {
	if (hourglass_sandflow >= 0) {
		hourglass_sandflow = (hourglass_sandflow + 1) % 3;
		if (hourglass_state >= 7) return;
		add_foretable(id_chtab_8_princessroom, hourglass_sandflow + 10, 20, 0, 164, blitters_10h_transp, 0);
	}
	if (hourglass_state) {
		add_midtable(id_chtab_8_princessroom, hourglass_state + 2, 19, 0, 168, blitters_10h_transp, 1);
	}
}

// seg001:08CA
void __pascal far reset_cutscene() {
	Guard.frame = 0;
	Kid.frame = 0;
	which_torch = 0;
	disable_keys = 0;
	hourglass_state = 0;
	// memset_near(byte_1ED6E, 0, 8); // not used elsewhere
	hourglass_sandflow = -1;
	cutscene_frame_time = 6;
	clear_tile_wipes();
	next_sound = -1;
}

// seg001:0908
void __pascal far do_flash(short color) {
	// stub
	if (color) {
		if (graphics_mode == gmMcgaVga) {
			set_bg_attr(0, color);
			if (color != 0) delay_ticks(2); // give some time to show the flash
		} else {
			// ...
		}
	}
}

void delay_ticks(Uint32 ticks) {
#ifdef USE_REPLAY
	if (replaying && skipping_replay) return;
#endif
	SDL_Delay(ticks *(1000/60));
}

// This flashes the background with the color specified, but does not add delay to show the flash
void do_flash_no_delay(short color) {
	if (color) set_bg_attr(0, color);
}

// seg001:0981
void __pascal far remove_flash() {
	// stub
	if (graphics_mode == gmMcgaVga) {
		set_bg_attr(0, 0);
	} else {
		// ...
	}
}

// seg001:09D7
void __pascal far end_sequence() {
	peel_type* peel;
	short bgcolor;
	short color;
	rect_type rect;
	short hof_index;
	short i;
	color = 0;
	bgcolor = 15;
	load_intro(1, &end_sequence_anim, 1);
	clear_screen_and_sounds();
	load_opt_sounds(sound_56_ending_music, sound_56_ending_music); // winning theme
	play_sound_from_buffer(sound_pointers[sound_56_ending_music]); // winning theme
	if(offscreen_surface) free_surface(offscreen_surface); // missing in original
	offscreen_surface = make_offscreen_buffer(&screen_rect);
	load_title_images(0);
	current_target_surface = offscreen_surface;
	draw_image_2(0 /*story frame*/, chtab_title40, 0, 0, 0);
	draw_image_2(3 /*The tyrant Jaffar*/, chtab_title40, 24, 25, get_text_color(15, color_15_brightwhite, 0x800));
	fade_in_2(offscreen_surface, 0x800);
	pop_wait(timer_0, 900);
	start_timer(timer_0, 240);
	draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, 0);
	transition_ltr();
	do_wait(timer_0);
	for (hof_index = 0; hof_index < hof_count; ++hof_index) {
		if (hof[hof_index].min < rem_min ||
			(hof[hof_index].min == rem_min && hof[hof_index].tick < rem_tick)
		) break;
	}
	if (hof_index < MAX_HOF_COUNT && hof_index <= hof_count) {
		fade_out_2(0x1000);
		for (i = 5; hof_index + 1 <= i; --i) {
			hof[i] = hof[i - 1];
		}
		hof[i].name[0] = 0;
		hof[i].min = rem_min;
		hof[i].tick = rem_tick;
		if (hof_count < MAX_HOF_COUNT) {
			++hof_count;
		}
		draw_image_2(0 /*story frame*/, chtab_title40, 0, 0, 0);
		draw_image_2(3 /*Prince Of Persia*/, chtab_title50, 24, 24, blitters_10h_transp);
		show_hof();
		offset4_rect_add(&rect, &hof_rects[hof_index], -4, -1, -40, -1);
		peel = read_peel_from_screen(&rect);
		if (graphics_mode == gmMcgaVga) {
			color = 0xBE;
			bgcolor = 0xB7;
		}
		draw_rect(&rect, bgcolor);
		fade_in_2(offscreen_surface, 0x1800);
		current_target_surface = onscreen_surface_;
		while(input_str(&rect, hof[hof_index].name, 24, "", 0, 4, color, bgcolor) <= 0);
		restore_peel(peel);
		show_hof_text(&hof_rects[hof_index], -1, 0, hof[hof_index].name);
		hof_write();
		pop_wait(timer_0, 120);
		current_target_surface = offscreen_surface;
		draw_image_2(0 /*main title image*/, chtab_title50, 0, 0, blitters_0_no_transp);
		transition_ltr();
	}
	while (check_sound_playing() && !key_test_quit()) idle();
	fade_out_2(0x1000);
	start_level = -1;
	start_game();
}

// seg001:0C94
void __pascal far expired() {
	if (!demo_mode) {
		if(offscreen_surface) free_surface(offscreen_surface); // missing in original
		offscreen_surface = NULL;
		clear_screen_and_sounds();
		offscreen_surface = make_offscreen_buffer(&screen_rect);
		load_intro(1, &time_expired, 1);
	}
	start_level = -1;
	start_game();
}

// seg001:0CCD
void __pascal far load_intro(int which_imgs,cutscene_ptr_type func,int free_sounds) {
	short current_star;
	draw_rect(&screen_rect, 0);
	if (free_sounds) {
		free_optional_sounds();
	}
	free_all_chtabs_from(id_chtab_3_princessinstory);
	load_chtab_from_file(id_chtab_8_princessroom, 950, "PV.DAT", 1<<13);
	load_chtab_from_file(id_chtab_9_princessbed, 980, "PV.DAT", 1<<14);
	current_target_surface = offscreen_surface;
	method_6_blit_img_to_scr(get_image(id_chtab_8_princessroom, 0), 0, 0, 0);
	method_6_blit_img_to_scr(get_image(id_chtab_9_princessbed, 0), 0, 142, blitters_2_or);

	// Free the images that are not needed anymore.
	free_all_chtabs_from(id_chtab_9_princessbed);
	SDL_FreeSurface(get_image(id_chtab_8_princessroom, 0));
	if (NULL != chtab_addrs[id_chtab_8_princessroom]) chtab_addrs[id_chtab_8_princessroom]->images[0] = NULL;

	load_chtab_from_file(id_chtab_3_princessinstory, 800, "PV.DAT", 1<<9);
	load_chtab_from_file(id_chtab_4_jaffarinstory_princessincutscenes,
	                     50*which_imgs + 850, "PV.DAT", 1<<10);
	for (current_star = 0; current_star < N_STARS; ++current_star) {
		draw_star(current_star, 0);
	}
	current_target_surface = onscreen_surface_;
	while (check_sound_playing()) {
		idle();
		do_paused();
	}
	need_drects = 1;
	reset_cutscene();
	is_cutscene = 1;
	func();
	is_cutscene = 0;
	free_all_chtabs_from(3);
	draw_rect(&screen_rect, 0);
}

typedef struct star_type {
	short x,y,color;
} star_type;

// data:0DC2
star_type stars[N_STARS] = {
	{20, 97,0},
	{16,104,1},
	{23,110,2},
	{17,116,3},
	{24,120,4},
	{18,128,0},
};
#define N_STAR_COLORS 5
// data:0DE6
const byte star_colors[N_STAR_COLORS] = {8, 7, 15, 15, 7};

// seg001:0E1C
void __pascal far draw_star(int which_star,int mark_dirty) {
	// The stars in the window of the princess's room.
	rect_type rect;
	short star_color;
	star_color = 15;
	rect.right = rect.left = stars[which_star].x;
	++rect.right;
	rect.bottom = rect.top = stars[which_star].y;
	++rect.bottom;
	if (graphics_mode != gmCga && graphics_mode != gmHgaHerc) {
		stars[which_star].color = (stars[which_star].color + 1) % N_STAR_COLORS;
		star_color = star_colors[stars[which_star].color];
	}
	draw_rect(&rect, star_color);
	if (mark_dirty) {
		add_drect(&rect);
	}
}

// seg001:0E94
void __pascal far show_hof() {
	// Hall of Fame
	short index;
	char time_text[12];
	for (index = 0; index < hof_count; ++index) {

#ifdef ALLOW_INFINITE_TIME
		int minutes, seconds;
		if (hof[index].min > 0) {
			minutes = hof[index].min - 1;
			seconds = hof[index].tick / 12;
		} else {
			// negative minutes means time ran 'forward' from 0:00 upwards
			minutes = abs(hof[index].min) - 1;
			seconds = (719 - hof[index].tick) / 12;
		}
		snprintf(time_text, sizeof(time_text), "%d:%02d", minutes, seconds);
#else
		snprintf(time_text, sizeof(time_text), "%d:%02d", hof[index].min - 1, hof[index].tick / 12);
#endif

		show_hof_text(&hof_rects[index], -1, 0, hof[index].name);
		show_hof_text(&hof_rects[index], 1, 0, time_text);
	}
	// stub
}

static const char* hof_file = "PRINCE.HOF";

const char* get_hof_path(char* custom_path_buffer, size_t max_len) {
	if (!use_custom_levelset) {
		return hof_file;
	}
	// if playing a custom levelset, try to use the mod folder
	snprintf(custom_path_buffer, max_len, "mods/%s/%s", levelset_name, hof_file /*PRINCE.HOF*/ );
	return custom_path_buffer;
}

// seg001:0F17
void __pascal far hof_write() {
	int handle;
	char custom_hof_path[POP_MAX_PATH];
	const char* hof_path = get_hof_path(custom_hof_path, sizeof(custom_hof_path));
	// no O_TRUNC
	handle = open(hof_path, O_WRONLY | O_CREAT | O_BINARY, 0600);
	if (handle < 0 ||
	    write(handle, &hof_count, 2) != 2 ||
	    write(handle, &hof, sizeof(hof)) != sizeof(hof) ||
	    close(handle))
		perror(hof_path);
	if (handle >= 0)
		close(handle);
}

// seg001:0F6C
void __pascal far hof_read() {
	int handle;
	hof_count = 0;
	char custom_hof_path[POP_MAX_PATH];
	const char* hof_path = get_hof_path(custom_hof_path, sizeof(custom_hof_path));
	handle = open(hof_path, O_RDONLY | O_BINARY);
	if (handle < 0)
		return;
	if (read(handle, &hof_count, 2) != 2 ||
	    read(handle, &hof, sizeof(hof)) != sizeof(hof)) {
		perror(hof_path);
		hof_count = 0;
	}
	close(handle);
}

// seg001:0FC3
void __pascal far show_hof_text(rect_type far *rect,int x_align,int y_align, const char *text) {
	short shadow_color;
	short text_color;
	rect_type rect2;
	text_color = 15;
	shadow_color = 0;
	if (graphics_mode == gmMcgaVga) {
		text_color = 0xB7;
	}
	offset2_rect(&rect2, rect, 1, 1);
	show_text_with_color(&rect2, x_align, y_align, text, shadow_color);
	show_text_with_color(rect, x_align, y_align, text, text_color);
}

// seg001:1029
int __pascal far fade_in_1() {
#ifdef USE_FADE
//	sbyte index;
	word interrupted;
	if (graphics_mode == gmMcgaVga) {
		fade_palette_buffer = make_pal_buffer_fadein(offscreen_surface, 0x6689, /*0*/ 2);
		is_global_fading = 1;
		do {
			interrupted = proc_cutscene_frame(1);
			if (interrupted == 1) {
				return 1;
			}
		} while (interrupted == 0);
		is_global_fading = 0;
	} else {
		// ...
	}
	return 0;
#else
	// stub
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, 0);
	updateScreen();
//	SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
	return 0;
#endif
}

// seg001:112D
int __pascal far fade_out_1() {
#ifdef USE_FADE
	word interrupted;
	if (graphics_mode == gmMcgaVga) {
		fade_palette_buffer = make_pal_buffer_fadeout(0x6689, /*0*/ 2);
		is_global_fading = 1;
		do {
			interrupted = proc_cutscene_frame(1);
			if (interrupted == 1) {
				return 1;
			}
		} while (interrupted == 0);
		is_global_fading = 0;
	} else {
		// ...
	}
#endif
	// stub
	return 0;
}
