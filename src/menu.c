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

#ifdef USE_MENU

byte arrowhead_up_image_data[];
byte arrowhead_down_image_data[];
byte arrowhead_left_image_data[];
byte arrowhead_right_image_data[];
image_type* arrowhead_up_image;
image_type* arrowhead_down_image;
image_type* arrowhead_left_image;
image_type* arrowhead_right_image;

void load_arrowhead_images() {
	// Make a dummy palette for decode_image().
	dat_pal_type dat_pal;
	memset(&dat_pal, 0, sizeof(dat_pal));
	dat_pal.vga[1].r = dat_pal.vga[1].g = dat_pal.vga[1].b = 0x3F; // white
	if (arrowhead_up_image == NULL) {
		arrowhead_up_image = decode_image((image_data_type*) arrowhead_up_image_data, &dat_pal);
	}
	if (arrowhead_down_image == NULL) {
		arrowhead_down_image = decode_image((image_data_type*) arrowhead_down_image_data, &dat_pal);
	}
//	dat_pal.vga[1] = vga_palette[color_7_lightgray];
	if (arrowhead_left_image == NULL) {
		arrowhead_left_image = decode_image((image_data_type*) arrowhead_left_image_data, &dat_pal);
	}
	if (arrowhead_right_image == NULL) {
		arrowhead_right_image = decode_image((image_data_type*) arrowhead_right_image_data, &dat_pal);
	}
}

#define MAX_MENU_ITEM_LENGTH 32

typedef struct pause_menu_item_type {
	int id;
	int previous, next;
	char text[MAX_MENU_ITEM_LENGTH];
} pause_menu_item_type;

enum pause_menu_item_ids {
	PAUSE_MENU_RESUME,
	PAUSE_MENU_SAVE_GAME,
	PAUSE_MENU_LOAD_GAME,
	PAUSE_MENU_RESTART_LEVEL,
	PAUSE_MENU_SETTINGS,
	PAUSE_MENU_QUIT_GAME,
	SETTINGS_MENU_GENERAL,
	SETTINGS_MENU_GAMEPLAY,
	SETTINGS_MENU_VISUALS,
	SETTINGS_MENU_MODS,
	SETTINGS_MENU_BACK,
};

pause_menu_item_type pause_menu_items[] = {
		{.id = PAUSE_MENU_RESUME,        .text = "RESUME"},
		{.id = PAUSE_MENU_SAVE_GAME,     .text = "SAVE GAME"},
		{.id = PAUSE_MENU_LOAD_GAME,     .text = "LOAD GAME"},
		{.id = PAUSE_MENU_RESTART_LEVEL, .text = "RESTART LEVEL"},
		{.id = PAUSE_MENU_SETTINGS,      .text = "SETTINGS"},
		{.id = PAUSE_MENU_QUIT_GAME,     .text = "QUIT GAME"}
};

int highlighted_pause_menu_item = PAUSE_MENU_RESUME;
int next_pause_menu_item;
int previous_pause_menu_item;
int drawn_menu;
byte pause_menu_alpha;

pause_menu_item_type settings_menu_items[] = {
		{.id = SETTINGS_MENU_GENERAL, .text = "GENERAL"},
		{.id = SETTINGS_MENU_GAMEPLAY, .text = "GAMEPLAY"},
		{.id = SETTINGS_MENU_VISUALS, .text = "VISUALS"},
		{.id = SETTINGS_MENU_MODS, .text = "MODS"},
		{.id = SETTINGS_MENU_BACK, .text = "BACK"},
};
int active_settings_subsection = 0;
int scroll_position = 0;
int menu_control_y;
int menu_control_x;

enum menu_setting_style_ids {
	SETTING_STYLE_TOGGLE = 0,
	SETTING_STYLE_SLIDER = 1,
	SETTING_STYLE_NUMBER = 2,
};

enum menu_setting_style_number_width_ids {
	SETTING_BYTE  = 0,
	SETTING_SBYTE = 1,
	SETTING_WORD  = 2,
	SETTING_SHORT = 3,
	//SETTING_DWORD = 4,
	SETTING_INT   = 5,
};

enum setting_ids {
	SETTING_ENABLE_INFO_SCREEN,
	SETTING_ENABLE_SOUND,
	SETTING_ENABLE_MUSIC,
	SETTING_ENABLE_CONTROLLER_RUMBLE,
	SETTING_JOYSTICK_THRESHOLD,
	SETTING_JOYSTICK_ONLY_HORIZONTAL,
	SETTING_FULLSCREEN,
	SETTING_USE_CORRECT_ASPECT_RATIO,
	SETTING_USE_INTEGER_SCALING,
	SETTING_ENABLE_FADE,
	SETTING_ENABLE_FLASH,
	SETTING_ENABLE_LIGHTING,
	SETTING_ENABLE_COPYPROT,
	SETTING_ENABLE_QUICKSAVE,
	SETTING_ENABLE_QUICKSAVE_PENALTY,
	SETTING_ENABLE_REPLAY,
	SETTING_USE_FIXES_AND_ENHANCEMENTS,
	SETTING_ENABLE_CROUCH_AFTER_CLIMBING,
	SETTING_ENABLE_FREEZE_TIME_DURING_END_MUSIC,
	SETTING_ENABLE_REMEMBER_GUARD_HP,
	SETTING_FIX_GATE_SOUNDS,
	SETTING_TWO_COLL_BUG,
	SETTING_FIX_INFINITE_DOWN_BUG,
	SETTING_FIX_GATE_DRAWING_BUG,
	SETTING_FIX_BIGPILLAR_CLIMB,
	SETTING_FIX_JUMP_DISTANCE_AT_EDGE,
	SETTING_FIX_EDGE_DISTANCE_CHECK_WHEN_CLIMBING,
	SETTING_FIX_PAINLESS_FALL_ON_GUARD,
	SETTING_FIX_WALL_BUMP_TRIGGERS_TILE_BELOW,
	SETTING_FIX_STAND_ON_THIN_AIR,
	SETTING_FIX_PRESS_THROUGH_CLOSED_GATES,
	SETTING_FIX_GRAB_FALLING_SPEED,
	SETTING_FIX_SKELETON_CHOMPER_BLOOD,
	SETTING_FIX_MOVE_AFTER_DRINK,
	SETTING_FIX_LOOSE_LEFT_OF_POTION,
	SETTING_FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES,
	SETTING_FIX_SAFE_LANDING_ON_SPIKES,
	SETTING_FIX_GLIDE_THROUGH_WALL,
	SETTING_FIX_DROP_THROUGH_TAPESTRY,
	SETTING_FIX_LAND_AGAINST_GATE_OR_TAPESTRY,
	SETTING_FIX_UNINTENDED_SWORD_STRIKE,
	SETTING_FIX_RETREAT_WITHOUT_LEAVING_ROOM,
	SETTING_FIX_RUNNING_JUMP_THROUGH_TAPESTRY,
	SETTING_FIX_PUSH_GUARD_INTO_WALL,
	SETTING_FIX_JUMP_THROUGH_WALL_ABOVE_GATE,
	SETTING_FIX_CHOMPERS_NOT_STARTING,
	SETTING_FIX_FEATHER_INTERRUPTED_BY_LEVELDOOR,
	SETTING_FIX_OFFSCREEN_GUARDS_DISAPPEARING,
};

typedef struct setting_type {
	int index;
	int id;
	int previous, next;
	byte style;
	byte number_type;
	void* linked;
	void* required;
	int min, max; // for sliders and number types
	char text[64];
	char explanation[256];
} setting_type;

setting_type general_settings[] = {
		{.id = SETTING_ENABLE_INFO_SCREEN, .style = SETTING_STYLE_TOGGLE, .linked = &enable_info_screen,
				.text = "Display info screen on launch",
				.explanation = "Display the SDLPoP information screen when the game starts."},
		{.id = SETTING_ENABLE_SOUND, .style = SETTING_STYLE_TOGGLE, .linked = &is_sound_on,
				.text = "Enable sound",
				.explanation = "Turn sound on or off."},
		{.id = SETTING_ENABLE_MUSIC, .style = SETTING_STYLE_TOGGLE, .linked = &enable_mixer,
				.text = "Enable music",
				.explanation = "Turn music on or off."},
		{.id = SETTING_ENABLE_CONTROLLER_RUMBLE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_controller_rumble,
				.text = "Enable controller rumble",
				.explanation = "If using a controller with a rumble motor, provide haptic feedback when the kid is hurt."},
		{.id = SETTING_JOYSTICK_THRESHOLD, .style = SETTING_STYLE_SLIDER,
				.linked = &joystick_threshold, .min = 0, .max = INT16_MAX,
				.text = "Joystick threshold",
				.explanation = "Joystick 'dead zone' sensitivity threshold."},
		{.id = SETTING_JOYSTICK_ONLY_HORIZONTAL, .style = SETTING_STYLE_TOGGLE, .linked = &joystick_only_horizontal,
				.text = "Horizontal joystick movement only",
				.explanation = "Use joysticks for horizontal movement only, not all-directional. "
						"This may make the game easier to control for some controllers."},
};

setting_type visuals_settings[] = {
		{.id = SETTING_FULLSCREEN, .style = SETTING_STYLE_TOGGLE, .linked = &start_fullscreen,
				.text = "Start fullscreen",
				.explanation = "Start the game in fullscreen mode.\nTo toggle fullscreen, press Alt+Enter."},
		{.id = SETTING_USE_CORRECT_ASPECT_RATIO, .style = SETTING_STYLE_TOGGLE, .linked = &use_correct_aspect_ratio,
				.text = "Use 4:3 aspect ratio",
				.explanation = "Render the game in the originally intended 4:3 aspect ratio."
				               "\nNB. Works best using a high resolution."},
		{.id = SETTING_USE_INTEGER_SCALING, .style = SETTING_STYLE_TOGGLE, .linked = &use_integer_scaling,
				.text = "Use integer scaling",
				.explanation = "Enable pixel perfect scaling. That is, make all pixels the same size by forcing integer scale factors.\n"
						"Combing with 4:3 aspect ratio requires at least 1600x1200."},
		{.id = SETTING_ENABLE_FADE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_fade,
				.text = "Fading enabled",
				.explanation = "Turn fading on or off."},
		{.id = SETTING_ENABLE_FLASH, .style = SETTING_STYLE_TOGGLE, .linked = &enable_flash,
				.text = "Flashing enabled",
				.explanation = "Turn flashing on or off."},
		{.id = SETTING_ENABLE_LIGHTING, .style = SETTING_STYLE_TOGGLE, .linked = &enable_lighting,
				.text = "Torch shadows enabled",
				.explanation = "Darken those parts of the screen that are not near a torch."},
};

setting_type gameplay_settings[] = {
		{.id = SETTING_ENABLE_COPYPROT, .style = SETTING_STYLE_TOGGLE, .linked = &enable_copyprot,
				.text = "Copy protection level",
				.explanation = "Enable or disable the potions (copy protection) level."},
		{.id = SETTING_ENABLE_QUICKSAVE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_quicksave,
				.text = "Enable quicksave",
				.explanation = "Enable quicksave/load feature.\nPress F6 to quicksave, F9 to quickload."},
		{.id = SETTING_ENABLE_QUICKSAVE_PENALTY, .style = SETTING_STYLE_TOGGLE, .linked = &enable_quicksave_penalty,
				.text = "Quicksave time penalty",
				.explanation = "Try to let time run out when quickloading (similar to dying).\n"
						"Actually, the 'remaining time' will still be restored, "
						"but a penalty (up to one minute) will be applied."},
		{.id = SETTING_ENABLE_REPLAY, .style = SETTING_STYLE_TOGGLE, .linked = &enable_replay,
				.text = "Enable replays",
				.explanation = "Enable recording/replay feature.\n"
						"Press Ctrl+Tab in-game to start recording.\n"
						"To stop, press Ctrl+Tab again."},
		{.id = SETTING_USE_FIXES_AND_ENHANCEMENTS, .style = SETTING_STYLE_TOGGLE, .linked = &use_fixes_and_enhancements,
				.text = "Enhanced mode (allow bug fixes)",
				.explanation = ""},
		{.id = SETTING_ENABLE_CROUCH_AFTER_CLIMBING, .style = SETTING_STYLE_TOGGLE,
				.linked = &enable_crouch_after_climbing, .required = &use_fixes_and_enhancements,
				.text = "Enable crouching after climbing",
				.explanation = "Adds a way to crouch immediately after climbing up: press down and forward simultaneously. "
						"In the original game, this could not be done (pressing down always causes the kid to climb down)."},
		{.id = SETTING_ENABLE_FREEZE_TIME_DURING_END_MUSIC, .style = SETTING_STYLE_TOGGLE,
				.linked = &enable_freeze_time_during_end_music, .required = &use_fixes_and_enhancements,
				.text = "Freeze time during level end music",
				.explanation = "Time runs out while the level ending music plays; however, the music can be skipped by disabling sound. "
						"This option stops time while the ending music is playing (so there is no need to disable sound)."},
		{.id = SETTING_ENABLE_REMEMBER_GUARD_HP, .style = SETTING_STYLE_TOGGLE,
				.linked = &enable_remember_guard_hp, .required = &use_fixes_and_enhancements,
				.text = "Remember guard hitpoints",
				.explanation = "Enable guard hitpoints not resetting to their default (maximum) value when re-entering the room."},
		{.id = SETTING_FIX_GATE_SOUNDS, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_gate_sounds, .required = &use_fixes_and_enhancements,
				.text = "Fix gate sounds bug",
				.explanation = "If a room is linked to itself on the left, the closing sounds of the gates in that room can't be heard."},
		{.id = SETTING_TWO_COLL_BUG, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_two_coll_bug, .required = &use_fixes_and_enhancements,
				.text = "Fix two collisions bug",
				.explanation = "An open gate or chomper may enable the Kid to go through walls. (Trick 7, 37, 62)"},
		{.id = SETTING_FIX_INFINITE_DOWN_BUG, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_infinite_down_bug, .required = &use_fixes_and_enhancements,
				.text = "Fix infinite down bug",
				.explanation = "If a room is linked to itself at the bottom, and the Kid's column has no floors, the game hangs."},
		{.id = SETTING_FIX_GATE_DRAWING_BUG, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_gate_drawing_bug, .required = &use_fixes_and_enhancements,
				.text = "Fix gate drawing bug",
				.explanation = "When a gate is under another gate, the top of the bottom gate is not visible."},
		{.id = SETTING_FIX_BIGPILLAR_CLIMB, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_bigpillar_climb, .required = &use_fixes_and_enhancements,
				.text = "Fix big pillar climbing bug",
				.explanation = "When climbing up to a floor with a big pillar top behind, turned right, Kid sees through floor."},
		{.id = SETTING_FIX_JUMP_DISTANCE_AT_EDGE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_jump_distance_at_edge, .required = &use_fixes_and_enhancements,
				.text = "Fix jump distance at edge",
				.explanation = "When climbing up two floors, turning around and jumping upward, the kid falls down. "
						"This fix makes the workaround of Trick 25 unnecessary."},
		{.id = SETTING_FIX_EDGE_DISTANCE_CHECK_WHEN_CLIMBING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_edge_distance_check_when_climbing, .required = &use_fixes_and_enhancements,
				.text = "Fix edge distance check when climbing",
				.explanation = "When climbing to a higher floor, the game unnecessarily checks how far away the edge below is. "
						"Sometimes you will \"teleport\" some distance when climbing from firm ground."},
		{.id = SETTING_FIX_PAINLESS_FALL_ON_GUARD, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_painless_fall_on_guard, .required = &use_fixes_and_enhancements,
				.text = "Fix painless fall on guard",
				.explanation = "Falling from a great height directly on top of guards does not hurt."},
		{.id = SETTING_FIX_WALL_BUMP_TRIGGERS_TILE_BELOW, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_wall_bump_triggers_tile_below, .required = &use_fixes_and_enhancements,
				.text = "Fix wall bump triggering tile below",
				.explanation = "Bumping against a wall may cause a loose floor below to drop, even though it has not been touched. (Trick 18, 34)"},
		{.id = SETTING_FIX_STAND_ON_THIN_AIR, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_stand_on_thin_air, .required = &use_fixes_and_enhancements,
				.text = "Fix standing on thin air",
				.explanation = "When pressing a loose tile, you can temporarily stand on thin air by standing up from crouching."},
		{.id = SETTING_FIX_PRESS_THROUGH_CLOSED_GATES, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_press_through_closed_gates, .required = &use_fixes_and_enhancements,
				.text = "Fix pressing through closed gates",
				.explanation = "Buttons directly to the right of gates can be pressed even though the gate is closed (Trick 1)"},
		{.id = SETTING_FIX_GRAB_FALLING_SPEED, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_grab_falling_speed, .required = &use_fixes_and_enhancements,
				.text = "Fix grab falling speed",
				.explanation = "By jumping and bumping into a wall, you can sometimes grab a ledge two stories down (which should not be possible)."},
		{.id = SETTING_FIX_SKELETON_CHOMPER_BLOOD, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_skeleton_chomper_blood, .required = &use_fixes_and_enhancements,
				.text = "Fix skeleton chomper blood",
				.explanation = "When chomped, skeletons cause the chomper to become bloody even though skeletons do not have blood."},
		{.id = SETTING_FIX_MOVE_AFTER_DRINK, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_move_after_drink, .required = &use_fixes_and_enhancements,
				.text = "Fix movement after drinking",
				.explanation = "Controls do not get released properly when drinking a potion, sometimes causing unintended movements."},
		{.id = SETTING_FIX_LOOSE_LEFT_OF_POTION, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_loose_left_of_potion, .required = &use_fixes_and_enhancements,
				.text = "Fix loose floor left of potion",
				.explanation = "A drawing bug occurs when a loose tile is placed to the left of a potion (or sword)."},
		{.id = SETTING_FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_guard_following_through_closed_gates, .required = &use_fixes_and_enhancements,
				.text = "Fix guards passing closed gates",
				.explanation = "Guards may \"follow\" the kid to the room on the left or right, even though there is a closed gate in between."},
		{.id = SETTING_FIX_SAFE_LANDING_ON_SPIKES, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_safe_landing_on_spikes, .required = &use_fixes_and_enhancements,
				.text = "Fix safe landing on spikes",
				.explanation = "When landing on the edge of a spikes tile, it is considered safe. (Trick 65)"},
		{.id = SETTING_FIX_GLIDE_THROUGH_WALL, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_glide_through_wall, .required = &use_fixes_and_enhancements,
				.text = "Fix gliding through walls",
				.explanation = "The kid may glide through walls after turning around while running (especially when weightless)."},
		{.id = SETTING_FIX_DROP_THROUGH_TAPESTRY, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_drop_through_tapestry, .required = &use_fixes_and_enhancements,
				.text = "Fix dropping through tapestries",
				.explanation = "The kid can drop down through a closed gate, when there is a tapestry (doortop) above the gate."},
		{.id = SETTING_FIX_LAND_AGAINST_GATE_OR_TAPESTRY, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_land_against_gate_or_tapestry, .required = &use_fixes_and_enhancements,
				.text = "Fix land against gate or tapestry",
				.explanation = "When dropping down and landing right in front of a wall, the entire landing animation should normally play. "
						"However, when falling against a closed gate or a tapestry(+floor) tile, the animation aborts."},
		{.id = SETTING_FIX_UNINTENDED_SWORD_STRIKE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_unintended_sword_strike, .required = &use_fixes_and_enhancements,
				.text = "Fix unintended sword strike",
				.explanation = "Sometimes, the kid may automatically strike immediately after drawing the sword. "
						"This especially happens when dropping down from a higher floor and then turning towards the opponent."},
		{.id = SETTING_FIX_RETREAT_WITHOUT_LEAVING_ROOM, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_retreat_without_leaving_room, .required = &use_fixes_and_enhancements,
				.text = "Fix retreat without leaving room",
				.explanation = "By repeatedly pressing 'back' in a swordfight, you can retreat out of a room without the room changing. (Trick 35)"},
		{.id = SETTING_FIX_RUNNING_JUMP_THROUGH_TAPESTRY, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_running_jump_through_tapestry, .required = &use_fixes_and_enhancements,
				.text = "Fix running jumps through tapestries",
				.explanation = "The kid can jump through a tapestry with a running jump to the left, if there is a floor above it."},
		{.id = SETTING_FIX_PUSH_GUARD_INTO_WALL, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_push_guard_into_wall, .required = &use_fixes_and_enhancements,
				.text = "Fix pushing guards into walls",
				.explanation = "Guards can be pushed into walls, because the game does not correctly check for walls located behind a guard."},
		{.id = SETTING_FIX_JUMP_THROUGH_WALL_ABOVE_GATE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_jump_through_wall_above_gate, .required = &use_fixes_and_enhancements,
				.text = "Fix jump through wall above gate",
				.explanation = "By doing a running jump into a wall, you can fall behind a closed gate two floors down. (e.g. skip in Level 7)"},
		{.id = SETTING_FIX_CHOMPERS_NOT_STARTING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_chompers_not_starting, .required = &use_fixes_and_enhancements,
				.text = "Fix chompers not starting",
				.explanation = "If you grab a ledge that is one or more floors down, the chompers on that row will not start."},
		{.id = SETTING_FIX_FEATHER_INTERRUPTED_BY_LEVELDOOR, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_feather_interrupted_by_leveldoor, .required = &use_fixes_and_enhancements,
				.text = "Fix feather fall interrupted by leveldoor",
				.explanation = "As soon as a level door has completely opened, the feather fall effect is interrupted because the sound stops."},
		{.id = SETTING_FIX_OFFSCREEN_GUARDS_DISAPPEARING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fix_offscreen_guards_disappearing, .required = &use_fixes_and_enhancements,
				.text = "Fix offscreen guards disappearing",
				.explanation = "Guards will often not reappear in another room if they have been pushed (partly or entirely) offscreen."},
};

enum {
	SETTING_START_MINUTES_LEFT,
	SETTING_START_TICKS_LEFT,
	SETTING_START_HITP,
	SETTING_MAX_HITP_ALLOWED,
	SETTING_SAVING_ALLOWED_FIRST_LEVEL,
	SETTING_SAVING_ALLOWED_LAST_LEVEL,
	SETTING_START_UPSIDE_DOWN,
	SETTING_START_IN_BLIND_MODE,
	SETTING_COPYPROT_LEVEL,
	SETTING_DRAWN_TILE_TOP_LEVEL_EDGE,
	SETTING_DRAWN_TILE_LEFT_LEVEL_EDGE,
	SETTING_LEVEL_EDGE_HIT_TILE,
	SETTING_ALLOW_TRIGGERING_ANY_TILE,
	SETTING_ENABLE_WDA_IN_PALACE,
	SETTING_FIRST_LEVEL,
	SETTING_SKIP_TITLE,
	SETTING_SHIFT_L_ALLOWED_UNTIL_LEVEL,
	SETTING_SHIFT_L_REDUCED_TICKS,


};

setting_type mods_settings[] = {
		{.id = SETTING_START_MINUTES_LEFT, .style = SETTING_STYLE_NUMBER,
				.linked = &start_minutes_left, .number_type = SETTING_SHORT, .min = -1, .max = INT16_MAX,
				.text = "Starting minutes left",
				.explanation = "Starting minutes left. (default = 60)\n"
						"To disable the time limit completely, set this to -1."},
		{.id = SETTING_START_TICKS_LEFT, .style = SETTING_STYLE_NUMBER,
				.linked = &start_ticks_left, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Starting ticks left",
				.explanation = "Starting number of ticks left in the first minute. (default = 719)\n"
						"1 tick = 1/12 second, so by default there are 59.92 seconds left in the first minute."},
		{.id = SETTING_START_HITP, .style = SETTING_STYLE_NUMBER,
				.linked = &start_hitp, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Starting hitpoints",
				.explanation = "Starting hitpoints. (default = 3)"},
		{.id = SETTING_MAX_HITP_ALLOWED, .style = SETTING_STYLE_NUMBER,
				.linked = &max_hitp_allowed, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Max hitpoints allowed",
				.explanation = "Maximum number of hitpoints you can get. (default = 10)"},
		{.id = SETTING_SAVING_ALLOWED_FIRST_LEVEL, .style = SETTING_STYLE_NUMBER,
				.linked = &saving_allowed_first_level, .number_type = SETTING_WORD, .max = 15,
				.text = "Saving allowed: first level",
				.explanation = "First level where you can save the game. (default = 3)"},
		{.id = SETTING_SAVING_ALLOWED_LAST_LEVEL, .style = SETTING_STYLE_NUMBER, .max = 15,
				.linked = &saving_allowed_last_level, .number_type = SETTING_WORD,
				.text = "Saving allowed: last level",
				.explanation = "Last level where you can save the game. (default = 13)"},
		{.id = SETTING_START_UPSIDE_DOWN, .style = SETTING_STYLE_TOGGLE, .linked = &start_upside_down,
				.text = "Start with the screen flipped",
				.explanation = "Start the game with the screen flipped upside down, similar to Shift+I (default = OFF)"},
		{.id = SETTING_START_IN_BLIND_MODE, .style = SETTING_STYLE_TOGGLE, .linked = &start_in_blind_mode,
				.text = "Start in blind mode",
				.explanation = "Start in blind mode, similar to Shift+B (default = OFF)"},
		{.id = SETTING_COPYPROT_LEVEL, .style = SETTING_STYLE_NUMBER,
				.linked = &copyprot_level, .number_type = SETTING_WORD, .max = 15,
				.text = "Copy protection before level",
				.explanation = "The potions level will appear before this level. Set to -1 to disable. (default = 2)"},
		{.id = SETTING_ALLOW_TRIGGERING_ANY_TILE, .style = SETTING_STYLE_TOGGLE, .linked = &allow_triggering_any_tile,
				.text = "Allow triggering any tile",
				.explanation = "Enable triggering any tile. For example a button could make loose floors fall, or start a stuck chomper. (default = OFF)"},
		{.id = SETTING_ENABLE_WDA_IN_PALACE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_wda_in_palace,
				.text = "Enable WDA in palace",
				.explanation = "Enable the dungeon wall drawing algorithm in the palace."
						"\nN.B. Use with a modified VPALACE.DAT that provides dungeon-like wall graphics!"},
		{.id = SETTING_FIRST_LEVEL, .style = SETTING_STYLE_NUMBER,
				.linked = &first_level, .number_type = SETTING_WORD, .max = 15,
				.text = "First level",
				.explanation = "Level that will be loaded when starting a new game."
						"\n(default = OFF)"},
		{.id = SETTING_SKIP_TITLE, .style = SETTING_STYLE_TOGGLE, .linked = &skip_title,
				.text = "Skip title sequence",
				.explanation = "Always skip the title sequence: the first level will be loaded immediately."
						"\n(default = OFF)"},
};

typedef struct settings_area_type {
	setting_type* settings;
	int setting_count;
} settings_area_type;

settings_area_type general_settings_area = { .settings = general_settings, .setting_count = COUNT(general_settings)};
settings_area_type gameplay_settings_area = { .settings = gameplay_settings, .setting_count = COUNT(gameplay_settings)};
settings_area_type visuals_settings_area = { .settings = visuals_settings, .setting_count = COUNT(visuals_settings)};
settings_area_type mods_settings_area = { .settings = mods_settings, .setting_count = COUNT(mods_settings)};

settings_area_type* get_settings_area(int menu_item_id) {
	switch(menu_item_id) {
		default:
			return NULL;
		case SETTINGS_MENU_GENERAL:
			return &general_settings_area;
		case SETTINGS_MENU_GAMEPLAY:
			return &gameplay_settings_area;
		case SETTINGS_MENU_VISUALS:
			return &visuals_settings_area;
		case SETTINGS_MENU_MODS:
			return &mods_settings_area;
	}
}

void init_pause_menu_items(pause_menu_item_type* first_item, int item_count) {
	if (item_count > 0) {
		for (int i = 0; i < item_count; ++i) {
			pause_menu_item_type* item = first_item + i;
			item->previous = (first_item + MAX(0, i-1))->id;
			item->next = (first_item + MIN(item_count-1, i+1))->id;
		}
		pause_menu_item_type* last_item = first_item + (item_count-1);
		first_item->previous = last_item->id;
		last_item->next = first_item->id;
	}
}

void init_settings_list(setting_type* first_setting, int setting_count) {
	if (setting_count > 0) {
		for (int i = 0; i < setting_count; ++i) {
			setting_type* item = first_setting + i;
			item->index = i;
			item->previous = (first_setting + MAX(0, i-1))->id;
			item->next = (first_setting + MIN(setting_count-1, i+1))->id;
		}
//		setting_type* last_item = first_setting + (setting_count-1);
//		first_setting->previous = last_item->id;
//		last_item->next = first_setting->id;
	}
}


void init_menu() {
	load_arrowhead_images();

	init_pause_menu_items(pause_menu_items, COUNT(pause_menu_items));
	init_pause_menu_items(settings_menu_items, COUNT(settings_menu_items));

	init_settings_list(general_settings, COUNT(general_settings));
	init_settings_list(visuals_settings, COUNT(visuals_settings));
	init_settings_list(gameplay_settings, COUNT(gameplay_settings));
	init_settings_list(mods_settings, COUNT(mods_settings));
}

bool is_mouse_over_rect(rect_type* rect) {
	return (mouse_x >= rect->left && mouse_x < rect->right && mouse_y >= rect->top && mouse_y < rect->bottom);
}

// Returns true if the mouse moved, false otherwise.
bool read_mouse_state() {
	float scale_x, scale_y;
	SDL_RenderGetScale(renderer_, &scale_x, &scale_y);
	int logical_width, logical_height;
	SDL_RenderGetLogicalSize(renderer_, &logical_width, &logical_height);
	int logical_scale_x = logical_width / 320; // These may be higher than 1, if 4:3 aspect ratio scaling is enabled.
	int logical_scale_y = logical_height / 200;
	scale_x *= logical_scale_x;
	scale_y *= logical_scale_y;
	if (!(scale_x > 0 && scale_y > 0 && logical_scale_x > 0 && logical_scale_y > 0)) return false;
	SDL_Rect viewport;
	SDL_RenderGetViewport(renderer_, &viewport); // Get the width/height of the 'black bars' around the rendering area.
	viewport.x /= logical_scale_x;
	viewport.y /= logical_scale_y;
	int last_mouse_x = mouse_x;
	int last_mouse_y = mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	mouse_x = (int) ((float)mouse_x/scale_x - viewport.x + 0.5f);
	mouse_y = (int) ((float)mouse_y/scale_y - viewport.y + 0.5f);
	bool mouse_moved = (last_mouse_x != mouse_x || last_mouse_y != mouse_y);
	return (mouse_moved || mouse_clicked);
}

rect_type explanation_rect = {170, 20, 200, 300};
int highlighted_setting_id = SETTING_ENABLE_INFO_SCREEN;
int controlled_area = 0;
int next_setting_id = 0;
int previous_setting_id = 0;
int at_scroll_up_boundary;
int at_scroll_down_boundary;



void enter_settings_subsection(int settings_menu_id, setting_type* settings) {
	if (active_settings_subsection != settings_menu_id) {
		highlighted_setting_id = settings[0].id;
	}
	active_settings_subsection = settings_menu_id;
	if (!mouse_state_changed) highlighted_pause_menu_item = 0;
	controlled_area = 1;
	scroll_position = 0;
}

void pause_menu_clicked(pause_menu_item_type* item) {
	//printf("Clicked option %s\n", item->text);
	clicked_or_pressed_enter = false; // prevent "click-through" because the screen changes
	play_sound(sound_22_loose_shake_3);
	play_next_sound();
	switch(item->id) {
		default: break;
		case PAUSE_MENU_RESUME:
			is_paused = 0;
			break;
		case PAUSE_MENU_SAVE_GAME:
			// TODO: Manual save games.
			last_key_scancode = SDL_SCANCODE_F6;
			break;
		case PAUSE_MENU_LOAD_GAME:
			// TODO: Manual save games.
			last_key_scancode = SDL_SCANCODE_F9;
			break;
		case PAUSE_MENU_RESTART_LEVEL:
			last_key_scancode = SDL_SCANCODE_A | WITH_CTRL;
			break;
		case PAUSE_MENU_SETTINGS:
			drawn_menu = 1;
			highlighted_pause_menu_item = SETTINGS_MENU_GENERAL;
			active_settings_subsection = 0;
			controlled_area = 0;
			break;
		case PAUSE_MENU_QUIT_GAME:
			last_key_scancode = SDL_SCANCODE_Q | WITH_CTRL;
			break;
		case SETTINGS_MENU_GENERAL:
			enter_settings_subsection(SETTINGS_MENU_GENERAL, general_settings);
			break;
		case SETTINGS_MENU_GAMEPLAY:
			enter_settings_subsection(SETTINGS_MENU_GAMEPLAY, gameplay_settings);
			break;
		case SETTINGS_MENU_VISUALS:
			enter_settings_subsection(SETTINGS_MENU_VISUALS, visuals_settings);
			break;
		case SETTINGS_MENU_MODS:
			enter_settings_subsection(SETTINGS_MENU_MODS, mods_settings);
			break;
		case SETTINGS_MENU_BACK:
			drawn_menu = 0;
			highlighted_pause_menu_item = PAUSE_MENU_RESUME;
			break;
	}
}

void draw_pause_menu_item(pause_menu_item_type* item, rect_type* parent, int* y_offset, int inactive_text_color) {
	rect_type text_rect = *parent;
	text_rect.top += *y_offset;
	int text_color = inactive_text_color;

	rect_type selection_box = text_rect;
	selection_box.bottom = selection_box.top + 8;
	selection_box.top -= 3;

	bool highlighted = (highlighted_pause_menu_item == item->id);
	if (mouse_state_changed && is_mouse_over_rect(&selection_box)) {
		highlighted_pause_menu_item = item->id;
		highlighted = true;
	}

	if (highlighted) {
		previous_pause_menu_item = item->previous;
		next_pause_menu_item = item->next;
		text_color = color_15_brightwhite;

		draw_rect_contours(&selection_box, color_7_lightgray);
#if 0 // do no unnecessary work...
		draw_rect_with_alpha(&selection_box, color_7_lightgray, 230);
		shrink2_rect(&selection_box, &selection_box, 1, 1);
		draw_rect_with_alpha(&selection_box, color_0_black, pause_menu_alpha);
#endif

		if (mouse_clicked) {
			if (is_mouse_over_rect(&selection_box)) {
				pause_menu_clicked(item);
			}
		} else if (clicked_or_pressed_enter == 1) {
			pause_menu_clicked(item);
		}

	}
	show_text_with_color(&text_rect, 0, -1, item->text, text_color);
	*y_offset += 13;

}

void draw_pause_menu() {
	pause_menu_alpha = 120;
	draw_rect_with_alpha(&rect_top, color_0_black, pause_menu_alpha);
	rect_type pause_rect_outer = {0, 110, 192, 210};
	rect_type pause_rect_inner;
	shrink2_rect(&pause_rect_inner, &pause_rect_outer, 5, 5);

	if (!mouse_state_changed) {
		if (menu_control_y == 1) {
			play_sound(sound_21_loose_shake_2);
			play_next_sound();
			highlighted_pause_menu_item = next_pause_menu_item;
		} else if (menu_control_y == -1) {
			play_sound(sound_21_loose_shake_2);
			play_next_sound();
			highlighted_pause_menu_item = previous_pause_menu_item;
		}
	}

	int y_offset = 50;
	for (int i = 0; i < COUNT(pause_menu_items); ++i) {
		draw_pause_menu_item(&pause_menu_items[i], &pause_rect_inner, &y_offset, color_15_brightwhite);
	}
}

void turn_setting_on(setting_type* setting) {
	play_sound(sound_10_sword_vs_sword);
	play_next_sound();
	switch(setting->id) {
		default:
			if (setting->linked != NULL) {
				*(byte*)(setting->linked) = 1;
			}
			break;
		case SETTING_USE_CORRECT_ASPECT_RATIO:
			use_correct_aspect_ratio = 1;
			apply_aspect_ratio();
			break;
		case SETTING_USE_INTEGER_SCALING:
			use_integer_scaling = 1;
			window_resized();
			break;
		case SETTING_ENABLE_LIGHTING:
			enable_lighting = 1;
			extern image_type* lighting_mask; // TODO: cleanup
			if (lighting_mask == NULL) {
				init_lighting();
			}
			need_full_redraw = 1;
			break;
		case SETTING_ENABLE_SOUND:
			turn_sound_on_off(15);
			break;
		case SETTING_USE_FIXES_AND_ENHANCEMENTS:
			turn_fixes_and_enhancements_on_off(1);
			break;
	}
}

void turn_setting_off(setting_type* setting) {
	play_sound(sound_10_sword_vs_sword);
	play_next_sound();
	switch(setting->id) {
		default:
			if (setting->linked != NULL) {
				*(byte*)(setting->linked) = 0;
			}
			break;
		case SETTING_USE_CORRECT_ASPECT_RATIO:
			use_correct_aspect_ratio = 0;
			apply_aspect_ratio();
			break;
		case SETTING_USE_INTEGER_SCALING:
			use_integer_scaling = 0;
			SDL_RenderSetIntegerScale(renderer_, SDL_FALSE);
			break;
		case SETTING_ENABLE_LIGHTING:
			enable_lighting = 0;
			need_full_redraw = 1;
			break;
		case SETTING_ENABLE_SOUND:
			turn_sound_on_off(0);
			break;
		case SETTING_USE_FIXES_AND_ENHANCEMENTS:
			use_fixes_and_enhancements = 0;
			turn_fixes_and_enhancements_on_off(0);
			break;
	}
}

int get_value(setting_type* setting) {
	int value = 0;
	if (setting->linked != NULL) {
		switch(setting->number_type) {
			default:
			case SETTING_BYTE:
				value = *(byte*) setting->linked;
				break;
			case SETTING_SBYTE:
				value = *(sbyte*) setting->linked;
				break;
			case SETTING_WORD:
				value = *(word*) setting->linked;
				break;
			case SETTING_SHORT:
				value = *(short*) setting->linked;
				break;
			case SETTING_INT:
				value = *(int*) setting->linked;
				break;
		}
	}
	return value;
}

void increase_setting(setting_type* setting, int old_value) {
	if (setting->linked != NULL && old_value < setting->max) {
		switch(setting->number_type) {
			default:
			case SETTING_BYTE:
				*(byte*) setting->linked += 1;
				break;
			case SETTING_SBYTE:
				*(sbyte*) setting->linked += 1;
				break;
			case SETTING_WORD:
				*(word*) setting->linked += 1;
				break;
			case SETTING_SHORT:
				*(short*) setting->linked += 1;
				break;
			case SETTING_INT:
				*(int*) setting->linked += 1;
				break;
		}
	}
}

void decrease_setting(setting_type* setting, int old_value) {
	if (setting->linked != NULL && old_value > setting->min) {
		switch(setting->number_type) {
			default:
			case SETTING_BYTE:
				*(byte*) setting->linked -= 1;
				break;
			case SETTING_SBYTE:
				*(sbyte*) setting->linked -= 1;
				break;
			case SETTING_WORD:
				*(word*) setting->linked -= 1;
				break;
			case SETTING_SHORT:
				*(short*) setting->linked -= 1;
				break;
			case SETTING_INT:
				*(int*) setting->linked -= 1;
				break;
		}
	}
}


void draw_setting_explanation(setting_type* setting) {
	show_text_with_color(&explanation_rect, 0, -1, setting->explanation, color_7_lightgray);
}

void draw_setting(setting_type* setting, rect_type* parent, int* y_offset, int inactive_text_color) {
	rect_type text_rect = *parent;
	text_rect.top += *y_offset;
	int text_color = inactive_text_color;
	int selected_color = color_15_brightwhite;
	int unselected_color = color_7_lightgray;

	rect_type setting_box = text_rect;
	setting_box.top -= 5;
	setting_box.bottom = setting_box.top + 15;
	setting_box.bottom = setting_box.top + 15;
	setting_box.left -= 10;
	setting_box.right += 10;

	if (mouse_clicked && is_mouse_over_rect(&setting_box)) {
		highlighted_setting_id = setting->id;
		controlled_area = 1;
	}

	if (highlighted_setting_id == setting->id) {
		next_setting_id = setting->next;
		previous_setting_id = setting->previous;
		at_scroll_up_boundary = (setting->index == scroll_position);
		at_scroll_down_boundary = (setting->index == scroll_position + 8);

		SDL_Rect dest_rect;
		rect_to_sdlrect(&setting_box, &dest_rect);
		uint32_t rgb_color = SDL_MapRGBA(overlay_surface->format, 55, 55, 55, 240);
		if (SDL_FillRect(overlay_surface, &dest_rect, rgb_color) != 0) {
			sdlperror("SDL_FillRect");
			quit(1);
		}
		rect_type left_side_of_setting_box = setting_box;
		left_side_of_setting_box.left = setting_box.left - 2;
		left_side_of_setting_box.right = setting_box.left;
		draw_rect_with_alpha(&left_side_of_setting_box, color_15_brightwhite, pause_menu_alpha);
		draw_setting_explanation(setting);
	}

	bool disabled = false;
	if (setting->required != NULL) {
		disabled = !(*(byte*)setting->required);
	}
	if (disabled) {
		text_color = color_7_lightgray;
	}

	show_text_with_color(&text_rect, -1, -1, setting->text, text_color);

	if (setting->style == SETTING_STYLE_TOGGLE && !disabled) {
		bool setting_enabled = true;
		if (setting->linked != NULL) {
			setting_enabled = *(byte*)setting->linked;
		}

		// Toggling the setting: either by clicking on "ON" or "OFF", or by pressing left/right.
		if (highlighted_setting_id == setting->id) {
			if (mouse_clicked) {
				if (setting_enabled) {
					rect_type OFF_hitbox = setting_box;
					OFF_hitbox.left = setting_box.right - 27;
					if (is_mouse_over_rect(&OFF_hitbox)) {
						turn_setting_off(setting);
						setting_enabled = false;
					}
				} else {
					rect_type ON_hitbox = setting_box;
					ON_hitbox.left = setting_box.right - 54;
					ON_hitbox.right = setting_box.right - 27;
					if (is_mouse_over_rect(&ON_hitbox)) {
						turn_setting_on(setting);
						setting_enabled = true;
					}
				}
			} else if (setting_enabled && menu_control_x > 0) {
				turn_setting_off(setting);
				setting_enabled = false;
			} else if (!setting_enabled && menu_control_x < 0) {
				turn_setting_on(setting);
				setting_enabled = true;
			}
		}

		int OFF_color = (setting_enabled) ? unselected_color : selected_color;
		int ON_color = (setting_enabled) ? selected_color : unselected_color;
		show_text_with_color(&text_rect, 1, -1, "OFF", OFF_color);
		text_rect.right -= 20;
		show_text_with_color(&text_rect, 1, -1, "ON", ON_color);

	} else if (setting->style == SETTING_STYLE_SLIDER) {
		int slider_value = 8000;
		float slider_position = 0.5f;
		rect_type slider_rect = text_rect;
		slider_rect.top += 1;
		slider_rect.left = slider_rect.right - 20;
		slider_rect.bottom = slider_rect.top + 3;
		draw_rect_contours(&slider_rect, color_7_lightgray);

		rect_type slider_pos_rect = slider_rect;
		int slider_center_x = (int) (0.5f + (float)slider_rect.left +
				slider_position*((float)(slider_rect.right - slider_rect.left)));
		slider_pos_rect.left = slider_center_x - 1;
		slider_pos_rect.right = slider_center_x + 1;
		slider_pos_rect.top -= 1;
		slider_pos_rect.bottom += 1;
		draw_rect_with_alpha(&slider_pos_rect, color_15_brightwhite, 255);

		text_rect.right -= 25;
		char value_text[16];
		snprintf(value_text, sizeof(value_text), "%d", slider_value);
		show_text_with_color(&text_rect, 1, -1, value_text, selected_color);

	} else if (setting->style == SETTING_STYLE_NUMBER) {
		int value = get_value(setting);
		if (highlighted_setting_id == setting->id) {
			if (mouse_clicked) {

				rect_type right_hitbox = {setting_box.top, text_rect.right - 5, setting_box.bottom, text_rect.right + 10};
				if (is_mouse_over_rect(&right_hitbox)) {
					increase_setting(setting, value);
				} else {
					char value_text[16];
					snprintf(value_text, sizeof(value_text), "%d", value);
					int value_text_width = get_line_width(value_text, strlen(value_text));
					rect_type left_hitbox = right_hitbox;
					left_hitbox.left -= (value_text_width + 10);
					left_hitbox.right -= (value_text_width + 5);
					if (is_mouse_over_rect(&left_hitbox)) {
						decrease_setting(setting, value);
					}
				}

			} else if (menu_control_x > 0) {
				increase_setting(setting, value);
			} else if (menu_control_x < 0) {
				decrease_setting(setting, value);
			}
		}

		value = get_value(setting); // May have been updated.
		char value_text[16];
		snprintf(value_text, sizeof(value_text), "%d", value);
		show_text_with_color(&text_rect, 1, -1, value_text, selected_color);

		if (highlighted_setting_id == setting->id) {
			int value_text_width = get_line_width(value_text, strlen(value_text));
			draw_image_transp_vga(arrowhead_right_image, text_rect.right + 2, text_rect.top);
			draw_image_transp_vga(arrowhead_left_image, text_rect.right - value_text_width - 6, text_rect.top);
		}

	}

	*y_offset += 15;
}

void menu_scroll(int y) {
	settings_area_type* current_settings_area = get_settings_area(active_settings_subsection);
	int max_scroll = MAX(0, current_settings_area->setting_count - 9);
	if (drawn_menu == 1 && controlled_area == 1) {
		if (y < 0 && scroll_position > 0) {
			--scroll_position;
		} else if (y > 0 && scroll_position < max_scroll) {
			++scroll_position;
		}
	}
}

void draw_settings_area(settings_area_type* settings_area) {
	if (settings_area == NULL) return;
	rect_type settings_area_rect = {0, 80, 170, 320};
	shrink2_rect(&settings_area_rect, &settings_area_rect, 20, 20);
	int y_offset = 0;
	int num_drawn_settings = 0;

	for (int i = 0; (i < settings_area->setting_count) && (num_drawn_settings < 9); ++i) {
		if (i >= scroll_position) {
			++num_drawn_settings;
			draw_setting(&settings_area->settings[i], &settings_area_rect, &y_offset, color_15_brightwhite);
		}
	}
	if (scroll_position > 0) {
		draw_image_transp_vga(arrowhead_up_image, 200, 10);
	}
	if (scroll_position + num_drawn_settings < settings_area->setting_count) {
		draw_image_transp_vga(arrowhead_down_image, 200, 151);
	}
}

void draw_settings_menu() {
	pause_menu_alpha = 230;
	draw_rect_with_alpha(&screen_rect, color_0_black, pause_menu_alpha);
	rect_type pause_rect_outer = {0, 10, 192, 80};
	rect_type pause_rect_inner;
	shrink2_rect(&pause_rect_inner, &pause_rect_outer, 5, 5);

	settings_area_type* settings_area = get_settings_area(active_settings_subsection);

	if (!mouse_state_changed) {
		if (menu_control_y == 1) {
			play_sound(sound_21_loose_shake_2);
			play_next_sound();
			if (controlled_area == 0) {
				highlighted_pause_menu_item = next_pause_menu_item;
			} else if (controlled_area == 1) {
				highlighted_setting_id = next_setting_id;
				if (at_scroll_down_boundary) {
					menu_scroll(1);
				}
			}
		} else if (menu_control_y == -1) {
			play_sound(sound_21_loose_shake_2);
			play_next_sound();
			if (controlled_area == 0) {
				highlighted_pause_menu_item = previous_pause_menu_item;
			} else if (controlled_area == 1) {
				highlighted_setting_id = previous_setting_id;
				if (at_scroll_up_boundary) {
					menu_scroll(-1);
				}
			}
		}
	}

	int y_offset = 50;
	for (int i = 0; i < COUNT(settings_menu_items); ++i) {
		pause_menu_item_type* item = &settings_menu_items[i];
		int text_color = (active_settings_subsection == item->id) ? color_15_brightwhite : color_7_lightgray;
		draw_pause_menu_item(&settings_menu_items[i], &pause_rect_inner, &y_offset, text_color);
	}

	draw_settings_area(settings_area);
}

void reset_paused_menu() {
	drawn_menu = 0;
	highlighted_pause_menu_item = PAUSE_MENU_RESUME;
}

void draw_pause_overlay() {
	if (!is_paused) return;
	mouse_state_changed = read_mouse_state();
	font_type* saved_font = textstate.ptr_font;
	textstate.ptr_font = &small_font;

	if (is_paused == 1) {
		is_paused = -1; // reset the menu if the menu is drawn for the first time
		reset_paused_menu();
	}

	if (drawn_menu == 0) {
		draw_pause_menu();
	} else if (drawn_menu == 1) {
		draw_settings_menu();
	}
	textstate.ptr_font = saved_font;
}

bool are_controller_buttons_released;

int key_test_paused_menu(int key) {
	if (is_joyst_mode) {
		if (joy_hat_states[0] == 0 && joy_hat_states[1] == 0 && joy_AY_buttons_state == 0 && joy_B_button_state == 0) {
			are_controller_buttons_released = true;
		} else if (are_controller_buttons_released) {
			are_controller_buttons_released = false;
			if (!(joy_hat_states[0] == 0 && joy_hat_states[1] == 0)) {
				menu_control_x = joy_hat_states[0];
				menu_control_y = joy_hat_states[1];
				return 0;
			}
			if (joy_AY_buttons_state == 1 /* A pressed */) {
				key = SDL_SCANCODE_RETURN;
			} else if (joy_B_button_state == 1) {
				key = SDL_SCANCODE_ESCAPE;
			}
		}
	}

	switch(key) {
		default:
			menu_control_y = 0;
			menu_control_x = 0;
			if (key & WITH_CTRL) {
				is_paused = 0;
				return key; // Allow Ctrl+R, etc.
			} else {
				return 0;
			}
		case SDL_SCANCODE_UP:
			menu_control_y = -1;
			menu_control_x = 0;
			return 0;
		case SDL_SCANCODE_DOWN:
			menu_control_y = 1;
			menu_control_x = 0;
			return 0;
		case SDL_SCANCODE_RIGHT:
			menu_control_y = 0;
			menu_control_x = 1;
			return 0;
		case SDL_SCANCODE_LEFT:
			menu_control_y = 0;
			menu_control_x = -1;
			return 0;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
			clicked_or_pressed_enter = 1;
			return 0;
		case SDL_SCANCODE_ESCAPE:
		case SDL_SCANCODE_BACKSPACE:
			if (drawn_menu == 1) {
				play_sound(sound_22_loose_shake_3);
				play_next_sound();
				if (controlled_area == 1) {
					controlled_area = 0;
					highlighted_pause_menu_item = active_settings_subsection;
					active_settings_subsection = 0;
					return 0;
				} else {
					reset_paused_menu();
					return 0;
				}
			}
			return key;
	}
}



// Small font (hardcoded).
// The alphanumeric characters were adapted from the freeware font '04b_03' by Yuji Oshimoto. See: http://www.04.jp.org/

#define BINARY_8(b7,b6,b5,b4,b3,b2,b1,b0) ((b0) | ((b1)<<1) | ((b2)<<2) | ((b3)<<3) | ((b4)<<4) | ((b5)<<5) | ((b6)<<6) | ((b7)<<7))
#define BINARY_4(b7,b6,b5,b4) (((b4)<<4) | ((b5)<<5) | ((b6)<<6) | ((b7)<<7))
#define _ 0
#define WORD(x) (byte)(x), (byte)((x)>>8)
#define IMAGE_DATA(height, width, flags) WORD(height), WORD(width), WORD(flags)

byte hc_small_font_data[] = {

		32, 126, WORD(5), WORD(2), WORD(1), WORD(1),

		// offsets (will be initialized at run-time)
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 41
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 51
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 61
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 71
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 81
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 91
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 101
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 111
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 121
		WORD(0), WORD(0), WORD(0), WORD(0), WORD(0), // 126

		IMAGE_DATA(1, 3, 1), // space
		BINARY_4( _,_,_,_ ),

		IMAGE_DATA(5, 1, 1), // !
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 3, 1), // "
		BINARY_4( 1,_,1,_ ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),

		IMAGE_DATA(5, 5, 1), // #
		BINARY_8( _,1,_,1,_,_,_,_ ),
		BINARY_8( 1,1,1,1,1,_,_,_ ),
		BINARY_8( _,1,_,1,_,_,_,_ ),
		BINARY_8( 1,1,1,1,1,_,_,_ ),
		BINARY_8( _,1,_,1,_,_,_,_ ),

		IMAGE_DATA(6, 3, 1), // $
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,1,_,_ ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 6, 1), // %
		BINARY_8( _,_,_,_,_,_,_,_ ),
		BINARY_8( 1,1,_,_,1,_,_,_ ),
		BINARY_8( 1,1,_,1,_,_,_,_ ),
		BINARY_8( _,_,1,_,1,1,_,_ ),
		BINARY_8( _,1,_,_,1,1,_,_ ),

		IMAGE_DATA(5, 5, 1), // &
		BINARY_8( _,1,1,_,_,_,_,_ ),
		BINARY_8( _,1,1,_,_,_,_,_ ),
		BINARY_8( 1,1,1,_,1,_,_,_ ),
		BINARY_8( 1,_,_,1,_,_,_,_ ),
		BINARY_8( _,1,1,_,1,_,_,_ ),

		IMAGE_DATA(2, 1, 1), // '
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 3, 1), // (
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 3, 1), // )
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(4, 5, 1),
		BINARY_8( _,_,_,_,_,_,_,_ ), // *
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( _,1,1,1,_,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),

		IMAGE_DATA(4, 3, 1), // +
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(6, 2, 1), // ,
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(3, 3, 1), // -
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 1, 1), // .
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // /
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // 0
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 2, 1), // 1
		BINARY_4( 1,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 4, 1), // 2
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // 3
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // 4
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( 1,1,1,1 ),
		BINARY_4( _,_,1,_ ),

		IMAGE_DATA(5, 4, 1), // 5
		BINARY_4( 1,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // 6
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // 7
		BINARY_4( 1,1,1,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 4, 1), // 8
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // 9
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 1, 1), // :
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(6, 2, 1), // ;
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 3, 1), // <
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,1,_ ),

		IMAGE_DATA(4, 3, 1), // =
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // >
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // ?
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,1,_ ),

		IMAGE_DATA(6, 4, 1), // @
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,1,1 ),
		BINARY_4( 1,_,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // A
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,1 ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // B
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // C
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // D
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // E
		BINARY_4( 1,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // F
		BINARY_4( 1,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // G
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,1,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // H
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 3, 1), // I
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // J
		BINARY_4( _,_,1,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // K
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( 1,1,_,_ ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // L
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,1 ),

		IMAGE_DATA(5, 5, 1), // M
		BINARY_8( 1,_,_,_,1,_,_,_ ),
		BINARY_8( 1,1,_,1,1,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( 1,_,_,_,1,_,_,_ ),
		BINARY_8( 1,_,_,_,1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // N
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,_,1 ),
		BINARY_4( 1,_,1,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // O
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // P
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(6, 4, 1), // Q
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( _,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // R
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // S
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 3, 1), // T
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 4, 1), // U
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // V
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 5, 1),
		BINARY_8( 1,_,_,_,1,_,_,_ ), // W
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( _,1,_,1,_,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // X
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // Y
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 3, 1), // Z
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 2, 1), // [
		BINARY_4( 1,1,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,_,_ ),

		IMAGE_DATA(5, 4, 1), // '\'
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // ]
		BINARY_4( 1,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,_,_ ),

		IMAGE_DATA(2, 3, 1), // ^
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,1,_ ),

		IMAGE_DATA(5, 3, 1), // _
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(2, 2, 1), // `
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 4, 1), // a
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // b
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 3, 1), // c
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // d
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // e
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,1,1 ),
		BINARY_4( 1,1,_,_ ),
		BINARY_4( _,1,1,1 ),

		IMAGE_DATA(5, 3, 1), // f
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(7, 4, 1), // g
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // h
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 1, 1), // i
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(7, 2, 1), // j
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // k
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 1, 1), // l
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 5, 1), // m
		BINARY_8( _,_,_,_,_,_,_,_ ),
		BINARY_8( 1,1,1,1,_,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // n
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),

		IMAGE_DATA(5, 4, 1), // o
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(7, 4, 1), // p
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(7, 4, 1), // q
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,_,_,1 ),

		IMAGE_DATA(5, 3, 1), // r
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( 1,1,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // s
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( 1,1,_,_ ),
		BINARY_4( _,_,1,1 ),
		BINARY_4( 1,1,1,_ ),

		IMAGE_DATA(5, 3, 1), // t
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,1,_ ),

		IMAGE_DATA(5, 4, 1), // u
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // v
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( _,1,_,_ ),

		IMAGE_DATA(5, 5, 1), // w
		BINARY_8( _,_,_,_,_,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( 1,_,1,_,1,_,_,_ ),
		BINARY_8( _,1,_,1,_,_,_,_ ),
		BINARY_8( _,1,_,1,_,_,_,_ ),

		IMAGE_DATA(5, 3, 1), // x
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,1,_ ),

		IMAGE_DATA(7, 4, 1), // y
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( 1,_,_,1 ),
		BINARY_4( _,1,1,1 ),
		BINARY_4( _,_,_,1 ),
		BINARY_4( _,1,1,_ ),

		IMAGE_DATA(5, 4, 1), // z
		BINARY_4( _,_,_,_ ),
		BINARY_4( 1,1,1,1 ),
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,1,1 ),

		IMAGE_DATA(5, 4, 1), // {
		BINARY_4( _,_,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,1,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,_,1,_ ),

		IMAGE_DATA(5, 1, 1), // |
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // }
		BINARY_4( 1,_,_,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( _,1,1,_ ),
		BINARY_4( _,1,_,_ ),
		BINARY_4( 1,_,_,_ ),

		IMAGE_DATA(5, 4, 1), // ~
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,1,_,1 ),
		BINARY_4( 1,_,1,_ ),
		BINARY_4( _,_,_,_ ),
		BINARY_4( _,_,_,_ ),


};

byte arrowhead_up_image_data[] = {
		IMAGE_DATA(4, 7, 1),
		BINARY_8( _,_,_,1,_,_,_,_ ),
		BINARY_8( _,_,1,1,1,_,_,_ ),
		BINARY_8( _,1,1,1,1,1,_,_ ),
		BINARY_8( 1,1,1,1,1,1,1,_ ),
};

byte arrowhead_down_image_data[] = {
		IMAGE_DATA(4, 7, 1),
		BINARY_8( 1,1,1,1,1,1,1,_ ),
		BINARY_8( _,1,1,1,1,1,_,_ ),
		BINARY_8( _,_,1,1,1,_,_,_ ),
		BINARY_8( _,_,_,1,_,_,_,_ ),
};

byte arrowhead_left_image_data[] = {
		IMAGE_DATA(5, 3, 1),
		BINARY_8( _,_,1,_,_,_,_,_ ),
		BINARY_8( _,1,1,_,_,_,_,_ ),
		BINARY_8( 1,1,1,_,_,_,_,_ ),
		BINARY_8( _,1,1,_,_,_,_,_ ),
		BINARY_8( _,_,1,_,_,_,_,_ ),
};

byte arrowhead_right_image_data[] = {
		IMAGE_DATA(5, 3, 1),
		BINARY_8( 1,_,_,_,_,_,_,_ ),
		BINARY_8( 1,1,_,_,_,_,_,_ ),
		BINARY_8( 1,1,1,_,_,_,_,_ ),
		BINARY_8( 1,1,_,_,_,_,_,_ ),
		BINARY_8( 1,_,_,_,_,_,_,_ ),
};

#endif //USE_MENU
