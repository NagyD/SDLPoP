/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2021  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#ifndef DATA_H
#define DATA_H

#ifdef BODY
// If included from data.c: definitions (without extern and with initialization).
#define INIT(...) __VA_ARGS__
#define extern
#else
// Otherwise: just declarations (with extern and without initialization).
#define INIT(...)
#undef extern
#endif

// data:5F8A
extern word text_time_remaining;
// data:4C56
extern word text_time_total;
// data:431C
extern word is_show_time;
// data:4C6E
extern word checkpoint;
// data:4E92
extern word upside_down;
// data:3D36
extern word resurrect_time;
// data:42B4
extern word dont_reset_time;
// data:4F7E
extern short rem_min;
// data:4F82
extern word rem_tick;
// data:4608
extern word hitp_beg_lev;
// data:4CAA
extern word need_level1_music;
// data:4380
extern surface_type* offscreen_surface;

// data:31E5
extern byte sound_flags INIT(= 0);
// data:295E
extern const rect_type screen_rect INIT(= {0, 0, 200, 320});
// data:3D12
extern word draw_mode;
// data:42B8
extern short start_level INIT(= -1);
// data:4CE6
extern byte * guard_palettes;
// data:4338
extern chtab_type *chtab_addrs[10];


#ifdef USE_COPYPROT
// data:4356
extern word copyprot_plac;
// data:3D16
extern word copyprot_idx;
// data:01CA
extern const char copyprot_letter[] INIT(= {'A','A','B','B','C','C','D','D','E','F','F','G','H','H','I','I','J','J','K','L','L','M','M','N','O','O','P','P','R','R','S','S','T','T','U','U','V','Y','W','Y'});
// data:4620
extern word cplevel_entr[14];
#endif
// data:46C6
extern dialog_type* copyprot_dialog;
// data:2944
extern dialog_settings_type dialog_settings
	INIT(= {
		add_dialog_rect,
		dialog_method_2_frame,
		4, 4, 4, 4, 3, 4, 1
	});
// data:2B76
extern rect_type dialog_rect_1 INIT(= {60, 56, 124, 264});
// data:2B7E
extern rect_type dialog_rect_2 INIT(= {61, 56, 120, 264});

// data:409E
extern word drawn_room;


// data:4CCD
extern byte curr_tile;
// data:4328
extern byte curr_modifier;


// data:4CD8
extern tile_and_mod leftroom_[3];
// data:5950
extern tile_and_mod row_below_left_[10];
// data:2274
extern const word tbl_line[] INIT(= {0, 10, 20});

// data:5966
extern word loaded_room;
// data:658A
extern byte* curr_room_tiles;
// data:5F88
extern byte* curr_room_modif;
// data:5968
extern word draw_xh;
// data:0F9E
extern word current_level INIT(= -1);
// data:3021
extern byte graphics_mode INIT(= 0);
// data:2BA6
#define VGA_PALETTE_DEFAULT { \
	{0x00, 0x00, 0x00},\
	{0x00, 0x00, 0x2A},\
	{0x00, 0x2A, 0x00},\
	{0x00, 0x2A, 0x2A},\
	{0x2A, 0x00, 0x00},\
	{0x2A, 0x00, 0x2A},\
	{0x2A, 0x15, 0x00},\
	{0x2A, 0x2A, 0x2A},\
	{0x15, 0x15, 0x15},\
	{0x15, 0x15, 0x3F},\
	{0x15, 0x3F, 0x15},\
	{0x15, 0x3F, 0x3F},\
	{0x3F, 0x15, 0x15},\
	{0x3F, 0x15, 0x3F},\
	{0x3F, 0x3F, 0x15},\
	{0x3F, 0x3F, 0x3F},}
// data:4CC0
extern word room_L;
// data:4CCE
extern word room_R;
// data:4C90
extern word room_A;
// data:4C96
extern word room_B;
// data:461A
extern word room_BR;
// data:43FE
extern word room_BL;
// data:4614
extern word room_AR;
// data:43DE
extern word room_AL;

// data:4F84
extern level_type level;

#ifdef USE_COLORED_TORCHES
extern byte torch_colors[24+1][30]; // indexed 1..24
#endif


// data:42AA
extern short table_counts[5];
#define backtable_count table_counts[0]
#define foretable_count table_counts[1]
#define wipetable_count table_counts[2]
#define  midtable_count table_counts[3]
#define  objtable_count table_counts[4]
// data:4D7E
extern short drects_count;
// data:434E
extern short peels_count;


// data:5FF4
extern back_table_type foretable[200];
// data:463C
extern back_table_type backtable[200];
// data:3D38
extern midtable_type midtable[50];
// data:5F1E
extern peel_type* peels_table[50];
// data:4D9A
extern rect_type drects[30];

// data:4CB8
extern sbyte obj_direction;
// data:2588
extern const byte chtab_flip_clip[10] INIT(= {1,0,1,1,1,1,0,0,0,0});
// data:42A6
extern short obj_clip_left;
// data:42C6
extern short obj_clip_top;
// data:42C0
extern short obj_clip_right;
// data:4082
extern short obj_clip_bottom;
// data:34D2
extern wipetable_type wipetable[300];
// data:2592
extern const byte chtab_shift[10] INIT(= {0,1,0,0,0,0,1,1,1,0});
// data:4354
extern word need_drects;
// data:4CC2
extern word is_blind_mode;

// data:0F86
extern const rect_type rect_top INIT(= {0, 0, 192, 320});
// data:0F96
extern const rect_type rect_bottom_text INIT(= {193, 70, 202, 250});

// data:4CB2
extern word leveldoor_right;
// data:4058
extern word leveldoor_ybottom;


// data:4CFA
extern byte palace_wall_colors[44*3];

// data:2942
extern word seed_was_init INIT(= 0);
// data:4084
extern dword random_seed;


// data:3010
extern surface_type* current_target_surface INIT(= NULL);

// data:4C5C
extern byte* doorlink2_ad;
// data:4C5A
extern byte* doorlink1_ad;



// data:4CC6
extern sbyte control_shift;
// data:461C
extern sbyte control_y;
// data:4612
extern sbyte control_x;

#ifdef USE_FADE
// data:4CCA
extern word is_global_fading;
// data:4400
extern palette_fade_type* fade_palette_buffer;
#endif
// data:4358
extern char_type Kid;
// data:295C
extern word is_keyboard_mode INIT(= 0);
// data:4E8A
extern word is_paused;
// data:42D0
extern word is_restart_level;
// data:31E4
extern byte sound_mode INIT(= 0);
// data:42C8
extern word is_joyst_mode;
// data:31E7
extern byte is_sound_on INIT(= 0x0F);
// data:3D18
extern word next_level;
// data:4C4A
extern short guardhp_delta;
// data:596A
extern word guardhp_curr;
// data:4CC8
extern word next_room;
// data:4C98
extern word hitp_curr;
// data:5FF2
extern word hitp_max;
// data:5FF0
extern short hitp_delta;
// data:4D94
extern word flash_color;
// data:4350
extern word flash_time;
// data:42DC
extern char_type Guard;

// data:437E
extern word need_quotes;
// data:4CF8
extern short roomleave_result;
// data:4D96
extern word different_room;
// data:4E94
extern sound_buffer_type* sound_pointers[58];
// data:4C58
extern word guardhp_max;
// data:405C
extern word is_feather_fall;
// data:4CBA
extern chtab_type* chtab_title40;
// data:4CD0
extern chtab_type* chtab_title50;
// data:405E
extern short hof_count;

#ifdef USE_SUPER_HIGH_JUMP
extern byte super_jump_timer INIT(=0);
extern byte super_jump_fall INIT(=0);
extern byte super_jump_room;
extern sbyte super_jump_col;
extern sbyte super_jump_row;
#endif

// data:009A
extern word demo_mode INIT(= 0);

// data:42CA
extern word is_cutscene;
extern bool is_ending_sequence; // added

// data:0FA0
extern cutscene_ptr_type tbl_cutscenes[16] INIT(= {
	NULL,
	NULL,
	cutscene_2_6,
	NULL,
	cutscene_4,
	NULL,
	cutscene_2_6,
	NULL,
	cutscene_8,
	cutscene_9,
	NULL,
	NULL,
	cutscene_12,
	NULL,
	NULL,
	NULL,
});

// data:408C
extern short mobs_count;
// data:4F7A
extern short trobs_count;
// data:4062
extern short next_sound;
// data:34AA
extern word grab_timer;
// data:594C
extern short can_guard_see_kid;
// data:594E
extern word holding_sword;
// data:4E90
extern short united_with_shadow;
// data:409C
extern word leveldoor_open;
// data:4610
extern word demo_index;
// data:4CD4
extern short demo_time;
// data:34A2
extern word have_sword;

// data:3D22
extern char_type Char;
// data:4D80
extern char_type Opp;


// data:42A2
extern short knock;
// data:4370
extern word is_guard_notice;

// data:656C
extern byte wipe_frames[30];
// data:4C72
extern sbyte wipe_heights[30];
// data:34AC
extern byte redraw_frames_anim[30];
// data:43E0
extern byte redraw_frames2[30];
// data:4C1A
extern byte redraw_frames_floor_overlay[30];
// data:4064
extern byte redraw_frames_full[30];
// data:5EFE
extern byte redraw_frames_fore[30];
// data:3484
extern byte tile_object_redraw[30];
// data:4C64
extern byte redraw_frames_above[10];
// data:4CE2
extern word need_full_redraw;
// data:588E
extern short n_curr_objs;
// data:5BAC
extern objtable_type objtable[50];
// data:5F8C
extern short curr_objs[50];

// data:4607
extern byte obj_xh;
// data:4616
extern byte obj_xl;
// data:4613
extern byte obj_y;
// data:4C9A
extern byte obj_chtab;
// data:42A4
extern byte obj_id;
// data:431E
extern byte obj_tilepos;
// data:4604
extern short obj_x;

// data:658C
extern frame_type cur_frame;
// data:5886
extern word seamless;
// data:4CBC
extern trob_type trob;
// data:4382
extern trob_type trobs[30];
// data:431A
extern short redraw_height;
// data:24DA
extern /*const*/ byte sound_interruptible[] INIT(= {
	0, // sound_0_fell_to_death
	1, // sound_1_falling
	1, // sound_2_tile_crashing
	1, // sound_3_button_pressed
	1, // sound_4_gate_closing
	1, // sound_5_gate_opening
	0, // sound_6_gate_closing_fast
	1, // sound_7_gate_stop
	1, // sound_8_bumped
	1, // sound_9_grab
	1, // sound_10_sword_vs_sword
	1, // sound_11_sword_moving
	1, // sound_12_guard_hurt
	1, // sound_13_kid_hurt
	0, // sound_14_leveldoor_closing
	0, // sound_15_leveldoor_sliding
	1, // sound_16_medium_land
	1, // sound_17_soft_land
	0, // sound_18_drink
	1, // sound_19_draw_sword
	1, // sound_20_loose_shake_1
	1, // sound_21_loose_shake_2
	1, // sound_22_loose_shake_3
	1, // sound_23_footstep
	0, // sound_24_death_regular
	0, // sound_25_presentation
	0, // sound_26_embrace
	0, // sound_27_cutscene_2_4_6_12
	0, // sound_28_death_in_fight
	1, // sound_29_meet_Jaffar
	0, // sound_30_big_potion
	0, // sound_31
	0, // sound_32_shadow_music
	0, // sound_33_small_potion
	0, // sound_34
	0, // sound_35_cutscene_8_9
	0, // sound_36_out_of_time
	0, // sound_37_victory
	0, // sound_38_blink
	0, // sound_39_low_weight
	0, // sound_40_cutscene_12_short_time
	0, // sound_41_end_level_music
	0, // sound_42
	0, // sound_43_victory_Jaffar
	0, // sound_44_skel_alive
	0, // sound_45_jump_through_mirror
	0, // sound_46_chomped
	1, // sound_47_chomper
	0, // sound_48_spiked
	0, // sound_49_spikes
	0, // sound_50_story_2_princess
	0, // sound_51_princess_door_opening
	0, // sound_52_story_4_Jaffar_leaves
	0, // sound_53_story_3_Jaffar_comes
	0, // sound_54_intro_music
	0, // sound_55_story_1_absence
	0, // sound_56_ending_music
	0
});
// data:42ED
extern byte curr_tilepos;
// data:432A
extern short curr_room;
// data:4CAC
extern mob_type curmob;
// data:4BB4
extern mob_type mobs[14];
// data:4332
extern short tile_col;
// data:229C
extern const short y_land[] INIT(= {-8, 55, 118, 181, 244});
// data:5888
extern word curr_guard_color;
// data:288C
extern byte key_states[SDL_NUM_SCANCODES];
// data:24A6
extern const byte x_bump[] INIT(= {-12, 2, 16, 30, 44, 58, 72, 86, 100, 114, 128, 142, 156, 170, 184, 198, 212, 226, 240, 254});
// data:42F4
extern word is_screaming;
// data:42EE
extern word offguard; // name from Apple II source
// data:3D32
extern word droppedout; // name from Apple II source

#ifdef USE_COPYPROT
// data:00A2
extern /*const*/ word copyprot_room[] INIT(= {3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4});
// data:00BE
extern const word copyprot_tile[] INIT(= {1,  5,  7,  9, 11, 21,  1,  3,  7, 11, 17, 21, 25, 27});
#endif

// data:5BAA
extern word exit_room_timer;
// data:4372
extern short char_col_right;
// data:5F86
extern short char_col_left;
// data:599C
extern short char_top_row;
// data:434C
extern short prev_char_top_row;
// data:432C
extern short prev_char_col_right;
// data:42CE
extern short prev_char_col_left;
// data:34A4
extern short char_bottom_row;
// data:3D34
extern short guard_notice_timer;
// data:42A0
extern short jumped_through_mirror;
// data:2292
extern const short y_clip[] INIT(= {-60, 3, 66, 129, 192});
// data:42F9
extern byte curr_tile2;
// data:4336
extern short tile_row;




// data:5F1C
extern word char_width_half;
// data:4618
extern word char_height;
// data:3D1C
extern short char_x_left;
// data:3D20
extern short char_x_left_coll;
// data:42F6
extern short char_x_right_coll;
// data:3D10
extern short char_x_right;
// data:4D98
extern short char_top_y;
// data:4096
extern byte fall_frame;
// data:4C0E
extern byte through_tile;
// data:5F82
extern sbyte infrontx; // name from Apple II source
// data:228E
extern const sbyte dir_front[] INIT(= {-1, 1});
// data:2290
extern const sbyte dir_behind[] INIT(= {1, -1});
// data:4320
extern word current_sound;
// data:4606
extern sbyte control_shift2;
// data:42A8
extern sbyte control_forward;
// data:4368
extern word guard_skill;
// data:4088
extern sbyte control_backward;
// data:4322
extern sbyte control_up;
// data:409A
extern sbyte control_down;
// data:4CE0
extern sbyte ctrl1_forward;
// data:4CD2
extern sbyte ctrl1_backward;
// data:4D92
extern sbyte ctrl1_up;
// data:4CD6
extern sbyte ctrl1_down;
// data:42F8
extern sbyte ctrl1_shift2;

// data:42F0
extern word shadow_initialized;

// data:4330
extern word guard_refrac;
// data:4098
extern word kid_sword_strike;


// data:6591
extern byte edge_type;



// data:596C
extern SDL_Surface* onscreen_surface_;
extern SDL_Surface* overlay_surface;
extern SDL_Surface* merged_surface;
extern SDL_Renderer* renderer_;
extern bool is_renderer_targettexture_supported;
extern SDL_Window* window_;
extern bool is_overlay_displayed;
extern SDL_Texture* texture_sharp;
extern SDL_Texture* texture_fuzzy;
extern SDL_Texture* texture_blurry;
extern SDL_Texture* target_texture;

extern SDL_GameController* sdl_controller_ INIT( = 0 );
extern SDL_Joystick* sdl_joystick_; // in case our joystick is not compatible with SDL_GameController
extern byte using_sdl_joystick_interface;
extern int joy_axis[6]; // hor/ver axes for left/right sticks + left and right triggers (in total 6 axes)
extern int joy_left_stick_states[2]; // horizontal, vertical
extern int joy_right_stick_states[2];
extern int joy_hat_states[2]; // horizontal, vertical
extern int joy_AY_buttons_state;
extern int joy_X_button_state;
extern int joy_B_button_state;
extern SDL_Haptic* sdl_haptic;

extern Uint64 perf_counters_per_tick;
extern Uint64 perf_frequency;
extern float milliseconds_per_counter;

extern char** sound_names;

extern int g_argc;
extern char** g_argv;


// data:405A
extern sbyte collision_row;
// data:42C2
extern sbyte prev_collision_row;

// data:4C10
extern sbyte prev_coll_room[10];
// data:4374
extern sbyte curr_row_coll_room[10];
// data:3D06
extern sbyte below_row_coll_room[10];
// data:42D2
extern sbyte above_row_coll_room[10];
// data:5890
extern byte curr_row_coll_flags[10];
// data:4CEA
extern byte above_row_coll_flags[10];
// data:4C4C
extern byte below_row_coll_flags[10];
// data:5BA0
extern byte prev_coll_flags[10];


// data:4F80
extern short pickup_obj_type;


// data:34CA
extern word justblocked; // name from Apple II source


// data:5F84
extern word last_loose_sound;

extern int last_key_scancode;
#ifdef USE_TEXT
extern font_type hc_font INIT(= {0x01,0xFF, 7,2,1,1, NULL});
extern textstate_type textstate INIT(= {0,0,0,15,&hc_font});
#endif
extern int need_quick_save INIT(= 0);
extern int need_quick_load INIT(= 0);

#ifdef USE_REPLAY
extern byte recording INIT(= 0);
extern byte replaying INIT(= 0);
extern dword num_replay_ticks INIT(= 0);
extern byte need_start_replay INIT(= 0);
extern byte need_replay_cycle INIT(= 0);
extern char replays_folder[POP_MAX_PATH] INIT(= "replays");
extern byte special_move;
extern dword saved_random_seed;
extern dword preserved_seed;
extern sbyte keep_last_seed;
extern byte skipping_replay;
extern byte replay_seek_target;
extern byte is_validate_mode;
extern dword curr_tick INIT(= 0);
#endif // USE_REPLAY

extern byte start_fullscreen INIT(= 0);
extern word pop_window_width INIT(= 640);
extern word pop_window_height INIT(= 400);
extern byte use_custom_levelset INIT(= 0);
extern char levelset_name[POP_MAX_PATH];
extern char mod_data_path[POP_MAX_PATH];
extern bool skip_mod_data_files;
extern bool skip_normal_data_files;

extern byte use_fixes_and_enhancements INIT(= 0);
extern byte enable_copyprot INIT(= 0);
extern byte enable_music INIT(= 1);
extern byte enable_fade INIT(= 1);
extern byte enable_flash INIT(= 1);
extern byte enable_text INIT(= 1);
extern byte enable_info_screen INIT(= 1);
extern byte enable_controller_rumble INIT(= 0);
extern byte joystick_only_horizontal INIT(= 0);
extern int joystick_threshold INIT(= 8000);
extern char gamecontrollerdb_file[POP_MAX_PATH] INIT(= "");
extern byte enable_quicksave INIT(= 1);
extern byte enable_quicksave_penalty INIT(= 1);
extern byte enable_replay INIT(= 1);
extern byte use_correct_aspect_ratio INIT(= 0);
extern byte use_integer_scaling INIT(= 0);
extern byte scaling_type INIT(= 0);
#ifdef USE_LIGHTING
extern byte enable_lighting INIT(= 0);
extern image_type* lighting_mask;
#endif
extern fixes_options_type fixes_saved;
extern fixes_options_type fixes_disabled_state;
extern fixes_options_type* fixes INIT(= &fixes_disabled_state);
extern byte use_custom_options;
extern custom_options_type custom_saved;
extern custom_options_type custom_defaults INIT(= {
		.start_minutes_left = 60,
		.start_ticks_left = 719,
		.start_hitp = 3,
		.max_hitp_allowed = 10,
		.saving_allowed_first_level = 3,
		.saving_allowed_last_level = 13,
		.start_upside_down = 0,
		.start_in_blind_mode = 0,
        // data:009E
        .copyprot_level = 2,
		.drawn_tile_top_level_edge = tiles_1_floor,
		.drawn_tile_left_level_edge = tiles_20_wall,
		.level_edge_hit_tile = tiles_20_wall,
		.allow_triggering_any_tile = 0,
		.enable_wda_in_palace = 0,
        .vga_palette = VGA_PALETTE_DEFAULT,
		.first_level = 1,
		.skip_title = 0,
		.shift_L_allowed_until_level = 4,
		.shift_L_reduced_minutes = 15,
		.shift_L_reduced_ticks  = 719,
		.demo_hitp = 4,
		.demo_end_room = 24,
		.intro_music_level = 1,
		.have_sword_from_level = 2,
		.checkpoint_level = 3,
		.checkpoint_respawn_dir = dir_FF_left,
		.checkpoint_respawn_room = 2,
		.checkpoint_respawn_tilepos = 6,
		.checkpoint_clear_tile_room = 7,
		.checkpoint_clear_tile_col = 4,
		.checkpoint_clear_tile_row = 0,
		.skeleton_level = 3,
		.skeleton_room = 1,
		.skeleton_trigger_column_1 = 2,
		.skeleton_trigger_column_2 = 3,
		.skeleton_column = 5,
		.skeleton_row = 1,
		.skeleton_require_open_level_door = 1,
		.skeleton_skill = 2,
		.skeleton_reappear_room = 3,
		.skeleton_reappear_x = 133,
		.skeleton_reappear_row = 1,
		.skeleton_reappear_dir = dir_0_right,
		.mirror_level = 4,
		.mirror_room = 4,
		.mirror_column = 4,
		.mirror_row = 0,
		.mirror_tile = tiles_13_mirror,
		.show_mirror_image = 1,
		.falling_exit_level = 6,
		.falling_exit_room = 1,
		.falling_entry_level = 7,
		.falling_entry_room = 17,
		.mouse_level = 8,
		.mouse_room = 16,
		.mouse_delay = 150,
		.mouse_object = 24,
		.mouse_start_x = 200,
		.loose_tiles_level = 13,
		.loose_tiles_room_1 = 23,
		.loose_tiles_room_2 = 16,
		.loose_tiles_first_tile = 22,
		.loose_tiles_last_tile = 27,
		.jaffar_victory_level = 13,
		.jaffar_victory_flash_time = 18,
		.hide_level_number_from_level = 14,
		.level_13_level_number = 12,
		.victory_stops_time_level = 13,
		.win_level = 14,
		.win_room = 5,
		.loose_floor_delay = 11,
		// data:02B2
		.tbl_level_type = {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0},
		// 1.3
        .tbl_level_color = {0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 0, 0, 3, 3, 4, 0},
		// data:03D4
        .tbl_guard_type = {0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 4, 3, -1, -1},
		// data:0EDA
		.tbl_guard_hp = {4, 3, 3, 3, 3, 4, 5, 4, 4, 5, 5, 5, 4, 6, 0, 0},
		.tbl_cutscenes_by_index = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		.tbl_entry_pose = {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
		.tbl_seamless_exit = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1},

		// guard skills
		.strikeprob    = { 61,100, 61, 61, 61, 40,100,220,  0, 48, 32, 48},
		.restrikeprob  = {  0,  0,  0,  5,  5,175, 16,  8,  0,255,255,150},
		.blockprob     = {  0,150,150,200,200,255,200,250,  0,255,255,255},
		.impblockprob  = {  0, 61, 61,100,100,145,100,250,  0,145,255,175},
		.advprob       = {255,200,200,200,255,255,200,  0,  0,255,100,100},
		.refractimer   = { 16, 16, 16, 16,  8,  8,  8,  8,  0,  8,  0,  0},
		.extrastrength = {  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0},

		// shadow's starting positions
		.init_shad_6 = {0x0F, 0x51, 0x76, 0, 0, 1, 0, 0},
		.init_shad_5  = {0x0F, 0x37, 0x37, 0, 0xFF, 0, 0, 0},
		.init_shad_12 = {0x0F, 0x51, 0xE8, 0, 0, 0, 0, 0},
		// automatic moves
		.demo_moves = {{0x00, 0}, {0x01, 1}, {0x0D, 0}, {0x1E, 1}, {0x25, 5}, {0x2F, 0}, {0x30, 1}, {0x41, 0}, {0x49, 2}, {0x4B, 0}, {0x63, 2}, {0x64, 0}, {0x73, 5}, {0x80, 6}, {0x88, 3}, {0x9D, 7}, {0x9E, 0}, {0x9F, 1}, {0xAB, 4}, {0xB1, 0}, {0xB2, 1}, {0xBC, 0}, {0xC1, 1}, {0xCD, 0}, {0xE9,-1}},
		.shad_drink_move = {{0x00, 0}, {0x01, 1}, {0x0E, 0}, {0x12, 6}, {0x1D, 7}, {0x2D, 2}, {0x31, 1}, {0xFF,-2}},

		// speeds
		.base_speed = 5,
		.fight_speed = 6,
		.chomper_speed = 15,
});
extern custom_options_type* custom INIT(= &custom_defaults);

extern full_image_type full_image[MAX_FULL_IMAGES] INIT(= {
        [TITLE_MAIN] =     { .id = 0, .chtab = &chtab_title50,
                             .blitter = blitters_0_no_transp,
                             .xpos = 0, .ypos = 0 },
        [TITLE_PRESENTS] = { .id = 1, .chtab = &chtab_title50,
                             .blitter = blitters_0_no_transp,
                             .xpos = 96, .ypos = 106 },
        [TITLE_GAME] =     { .id = 2, .chtab = &chtab_title50,
                             .blitter = blitters_0_no_transp,
                             .xpos = 96, .ypos = 122 },
        [TITLE_POP] =      { .id = 3, .chtab = &chtab_title50,
                             .blitter = blitters_10h_transp,
                             .xpos = 24, .ypos = 107 },
        [TITLE_MECHNER] =  { .id = 4, .chtab = &chtab_title50,
                             .blitter = blitters_0_no_transp,
                             .xpos = 48, .ypos = 184 },
        [HOF_POP] =        { .id = 3, .chtab = &chtab_title50,
                             .blitter = blitters_10h_transp,
                             .xpos = 24, .ypos = 24 },
        [STORY_FRAME] =    { .id = 0, .chtab = &chtab_title40,
                             .blitter = blitters_0_no_transp,
                             .xpos = 0, .ypos = 0 },
        [STORY_ABSENCE] =  { .id = 1, .chtab = &chtab_title40,
                             .blitter = blitters_white,
                             .xpos = 24, .ypos = 25 },
        [STORY_MARRY] =    { .id = 2, .chtab = &chtab_title40,
                             .blitter = blitters_white,
                             .xpos = 24, .ypos = 25 },
        [STORY_HAIL] =     { .id = 3, .chtab = &chtab_title40,
                             .blitter = blitters_white,
                             .xpos = 24, .ypos = 25 },
        [STORY_CREDITS] =  { .id = 4, .chtab = &chtab_title40,
                             .blitter = blitters_white,
                             .xpos = 24, .ypos = 26 },
});

// data:009C
extern word cheats_enabled INIT(= 0);
#ifdef USE_DEBUG_CHEATS
extern byte debug_cheats_enabled INIT(= 0);
extern byte is_timer_displayed INIT(= 0);
extern byte is_feather_timer_displayed INIT(= 0);
#endif

#ifdef USE_MENU
extern font_type hc_small_font INIT(= {32, 126, 5, 2, 1, 1, NULL});
extern bool have_mouse_input;
extern bool have_keyboard_or_controller_input;
extern int mouse_x, mouse_y;
extern bool mouse_moved;
extern bool mouse_clicked;
extern bool mouse_button_clicked_right;
extern bool pressed_enter;
extern bool escape_key_suppressed;
extern int menu_control_scroll_y;
extern sbyte is_menu_shown;
extern byte enable_pause_menu INIT(= 1);
#endif
extern char mods_folder[POP_MAX_PATH] INIT(= "mods");

extern int play_demo_level;

#ifdef USE_REPLAY
extern int g_deprecation_number;
#endif

#undef INIT
#undef extern

#endif
