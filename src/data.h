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
extern word start_level;
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

// data:02B2
extern /*const*/ byte tbl_level_type[16] INIT(= {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0});
// 1.3
extern /*const*/ word tbl_level_color[16] INIT(= {0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 0, 0, 3, 3, 4, 0});
// data:0F9E
extern word current_level INIT(= -1);
// data:3021
extern byte graphics_mode INIT(= 0);
// data:2BA6
extern rgb_type vga_palette[16] INIT(= {
		{0x00, 0x00, 0x00},
		{0x00, 0x00, 0x2A},
		{0x00, 0x2A, 0x00},
		{0x00, 0x2A, 0x2A},
		{0x2A, 0x00, 0x00},
		{0x2A, 0x00, 0x2A},
		{0x2A, 0x15, 0x00},
		{0x2A, 0x2A, 0x2A},
		{0x15, 0x15, 0x15},
		{0x15, 0x15, 0x3F},
		{0x15, 0x3F, 0x15},
		{0x15, 0x3F, 0x3F},
		{0x3F, 0x15, 0x15},
		{0x3F, 0x15, 0x3F},
		{0x3F, 0x3F, 0x15},
		{0x3F, 0x3F, 0x3F},
});

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
#ifdef USE_COPYPROT
// data:009E
extern word copyprot_level INIT(= 2);
#endif
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


// data:009A
extern word demo_mode INIT(= 0);

// data:42CA
extern word is_cutscene;

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
extern /*const*/ byte sound_interruptible[] INIT(= {0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
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
// data:03D4
extern /*const*/ short tbl_guard_type[16] INIT(= {0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 4, 3, -1, -1});
// data:0EDA
extern /*const*/ byte tbl_guard_hp[16] INIT(= {4, 3, 3, 3, 3, 4, 5, 4, 4, 5, 5, 5, 4, 6, 0, 0});
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
extern SDL_Renderer* renderer_;
extern SDL_Window* window_;
extern SDL_Texture* sdl_texture_;

extern SDL_GameController* sdl_controller_ INIT( = 0 );
extern int joy_axis[6]; // hor/ver axes for left/right sticks + left and right triggers (in total 6 axes)
extern int joy_left_stick_states[2]; // horizontal, vertical
extern int joy_right_stick_states[2];
extern int joy_hat_states[2]; // horizontal, vertical
extern int joy_AY_buttons_state;
extern int joy_X_button_state;
extern int joy_B_button_state;
extern SDL_Haptic* sdl_haptic;

extern int screen_updates_suspended;

#ifdef USE_MIXER
extern char** sound_names;
#endif

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

// data:4CE2
word need_full_redraw;

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
#endif // USE_REPLAY

extern byte start_fullscreen INIT(= 0);
extern word pop_window_width INIT(= 640);
extern word pop_window_height INIT(= 400);
extern byte use_custom_levelset INIT(= 0);
extern char levelset_name[POP_MAX_PATH];

extern byte use_fixes_and_enhancements INIT(= 0);
extern byte enable_copyprot INIT(= 0);
extern byte enable_mixer INIT(= 1);
extern byte enable_fade INIT(= 1);
extern byte enable_flash INIT(= 1);
extern byte enable_text INIT(= 1);
extern byte enable_info_screen INIT(= 1);
extern byte enable_controller_rumble INIT(= 0);
extern byte joystick_only_horizontal INIT(= 0);
extern byte enable_quicksave INIT(= 1);
extern byte enable_quicksave_penalty INIT(= 1);
extern byte enable_replay INIT(= 1);
extern byte enable_crouch_after_climbing INIT(= 1);
extern byte enable_freeze_time_during_end_music INIT(= 1);
extern byte fix_gate_sounds INIT(= 1);
extern byte fix_two_coll_bug INIT(= 1);
extern byte fix_infinite_down_bug INIT(= 1);
extern byte fix_gate_drawing_bug INIT(= 0);
extern byte fix_bigpillar_climb INIT(= 0);
extern byte fix_jump_distance_at_edge INIT(= 1);
extern byte fix_edge_distance_check_when_climbing INIT(= 1);
extern byte fix_painless_fall_on_guard INIT(= 1);
extern byte fix_wall_bump_triggers_tile_below INIT(= 1);
extern byte fix_stand_on_thin_air INIT(= 1);
extern byte fix_press_through_closed_gates INIT(= 1);
extern byte fix_grab_falling_speed INIT(= 1);
extern byte fix_skeleton_chomper_blood INIT(= 1);
extern byte fix_move_after_drink INIT(= 1);
extern byte fix_loose_left_of_potion INIT(= 1);
extern byte fix_guard_following_through_closed_gates INIT(= 1);
extern byte fix_safe_landing_on_spikes INIT(= 1);
extern byte use_correct_aspect_ratio INIT(= 0);
extern byte enable_remember_guard_hp INIT(= 1);
extern byte fix_glide_through_wall INIT(= 1);
extern byte fix_drop_through_tapestry INIT(= 1);
extern byte fix_land_against_gate_or_tapestry INIT(= 1);
extern byte fix_unintended_sword_strike INIT(= 1);
extern byte fix_retreat_without_leaving_room INIT(= 1);
extern byte fix_running_jump_through_tapestry INIT(= 1);
extern byte fix_push_guard_into_wall INIT(= 1);
extern byte fix_jump_through_wall_above_gate INIT(= 1);
extern byte fix_chompers_not_starting INIT(= 1);
extern byte fix_feather_interrupted_by_leveldoor INIT(= 1);
extern byte fix_offscreen_guards_disappearing INIT(= 1);
extern byte fix_move_after_sheathe INIT(= 1);

// Custom Gameplay settings
extern word start_minutes_left INIT(= 60);
extern word start_ticks_left INIT(= 719);
extern word start_hitp INIT(= 3);
extern word max_hitp_allowed INIT(= 10);
extern word saving_allowed_first_level INIT(= 3);
extern word saving_allowed_last_level INIT(= 13);
extern byte start_upside_down INIT(= 0);
extern byte start_in_blind_mode INIT(= 0);
extern byte drawn_tile_top_level_edge INIT(= tiles_1_floor);
extern byte drawn_tile_left_level_edge INIT(= tiles_20_wall);
extern byte level_edge_hit_tile INIT(= tiles_20_wall);
extern byte allow_triggering_any_tile INIT(= 0);
extern byte enable_wda_in_palace INIT(= 0);
extern word first_level INIT(= 1);
extern byte skip_title INIT(= 0);
extern word shift_L_allowed_until_level INIT(= 4);
extern word shift_L_reduced_minutes INIT(= 15);
extern word shift_L_reduced_ticks INIT(= 719);

#ifdef USE_DEBUG_CHEATS
extern byte debug_cheats_enabled INIT(= 0);
extern const rect_type timer_rect INIT(= {1, 2, 8, 55});
extern byte is_timer_displayed INIT(= 0);
#endif

// customized cutscene set-up: handled as index into a lookup table (can't rely on function pointers being stable!)
extern byte tbl_cutscenes_by_index[16] INIT(= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
extern cutscene_ptr_type tbl_cutscenes_lookup[16] INIT(= {
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

#undef INIT
#undef extern

#endif
