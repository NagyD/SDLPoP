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

typedef struct pause_menu_item_type pause_menu_item_type;
struct pause_menu_item_type {
	int id;
	pause_menu_item_type* previous;
	pause_menu_item_type* next;
	void* required;
	char text[MAX_MENU_ITEM_LENGTH];
};

enum pause_menu_item_ids {
	PAUSE_MENU_RESUME,
	PAUSE_MENU_CHEATS,
	PAUSE_MENU_SAVE_GAME,
	PAUSE_MENU_LOAD_GAME,
	PAUSE_MENU_RESTART_LEVEL,
	PAUSE_MENU_SETTINGS,
	PAUSE_MENU_RESTART_GAME,
	PAUSE_MENU_QUIT_GAME,
	SETTINGS_MENU_GENERAL,
	SETTINGS_MENU_GAMEPLAY,
	SETTINGS_MENU_VISUALS,
	SETTINGS_MENU_MODS,
	SETTINGS_MENU_LEVEL_CUSTOMIZATION,
	SETTINGS_MENU_BACK,
};

pause_menu_item_type pause_menu_items[] = {
		{.id = PAUSE_MENU_RESUME,        .text = "RESUME"},
		// TODO: Add a cheats menu, where you can choose a cheat from a list?
		/*{.id = PAUSE_MENU_CHEATS,        .text = "CHEATS", .required = &cheats_enabled},*/
#ifdef USE_QUICKSAVE // TODO: If quicksave is disabled, show regular save/load instead?
		{.id = PAUSE_MENU_SAVE_GAME,     .text = "QUICKSAVE"},
		{.id = PAUSE_MENU_LOAD_GAME,     .text = "QUICKLOAD"},
#endif
		{.id = PAUSE_MENU_RESTART_LEVEL, .text = "RESTART LEVEL"},
		{.id = PAUSE_MENU_SETTINGS,      .text = "SETTINGS"},
		{.id = PAUSE_MENU_RESTART_GAME,  .text = "RESTART GAME"},
		{.id = PAUSE_MENU_QUIT_GAME,     .text = "QUIT GAME"},
};

int hovering_pause_menu_item = PAUSE_MENU_RESUME;
pause_menu_item_type* next_pause_menu_item;
pause_menu_item_type* previous_pause_menu_item;
int drawn_menu;
byte pause_menu_alpha;
int current_dialog_box;
const char* current_dialog_text;
word menu_current_level = 1;
bool need_close_menu;

enum menu_dialog_ids {
	DIALOG_NONE,
	DIALOG_RESTORE_DEFAULT_SETTINGS,
	DIALOG_CONFIRM_QUIT,
	DIALOG_SELECT_LEVEL,
};

pause_menu_item_type settings_menu_items[] = {
		{.id = SETTINGS_MENU_GENERAL, .text = "GENERAL"},
		{.id = SETTINGS_MENU_GAMEPLAY, .text = "GAMEPLAY"},
		{.id = SETTINGS_MENU_VISUALS, .text = "VISUALS"},
		{.id = SETTINGS_MENU_MODS, .text = "MODS"},
		{.id = SETTINGS_MENU_BACK, .text = "BACK"},
};
int active_settings_subsection = 0;
int highlighted_settings_subsection = 0;
int scroll_position = 0;
int menu_control_y;
int menu_control_x;
int menu_control_back;

enum menu_setting_style_ids {
	SETTING_STYLE_TOGGLE,
	SETTING_STYLE_NUMBER,
	SETTING_STYLE_TEXT_ONLY,
};

enum menu_setting_number_type_ids {
	SETTING_BYTE  = 0,
	SETTING_SBYTE = 1,
	SETTING_WORD  = 2,
	SETTING_SHORT = 3,
	//SETTING_DWORD = 4,
	SETTING_INT   = 5,
};

enum setting_ids {
	SETTING_RESET_ALL_SETTINGS,
	SETTING_SHOW_MENU_ON_PAUSE,
	SETTING_ENABLE_INFO_SCREEN,
	SETTING_ENABLE_SOUND,
	SETTING_ENABLE_MUSIC,
	SETTING_ENABLE_CONTROLLER_RUMBLE,
	SETTING_JOYSTICK_THRESHOLD,
	SETTING_JOYSTICK_ONLY_HORIZONTAL,
	SETTING_FULLSCREEN,
	SETTING_USE_CORRECT_ASPECT_RATIO,
	SETTING_USE_INTEGER_SCALING,
	SETTING_SCALING_TYPE,
	SETTING_ENABLE_FADE,
	SETTING_ENABLE_FLASH,
	SETTING_ENABLE_LIGHTING,
	SETTING_ENABLE_CHEATS,
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
	SETTING_FIX_MOVE_AFTER_SHEATHE,
	SETTING_FIX_HIDDEN_FLOORS_DURING_FLASHING,
	SETTING_FIX_HANG_ON_TELEPORT,
	SETTING_FIX_EXIT_DOOR,
	SETTING_FIX_QUICKSAVE_DURING_FEATHER,
	SETTING_FIX_CAPED_PRINCE_SLIDING_THROUGH_GATE,
	SETTING_FIX_DOORTOP_DISABLING_GUARD,
	SETTING_enable_super_high_jump,
	SETTING_USE_CUSTOM_OPTIONS,
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
	SETTING_SHIFT_L_REDUCED_MINUTES,
	SETTING_SHIFT_L_REDUCED_TICKS,
	SETTING_DEMO_HITP,
	SETTING_DEMO_END_ROOM,
	SETTING_INTRO_MUSIC_LEVEL,
	SETTING_HAVE_SWORD_FROM_LEVEL,
	SETTING_CHECKPOINT_LEVEL,
	SETTING_CHECKPOINT_RESPAWN_DIR,
	SETTING_CHECKPOINT_RESPAWN_ROOM,
	SETTING_CHECKPOINT_RESPAWN_TILEPOS,
	SETTING_CHECKPOINT_CLEAR_TILE_ROOM,
	SETTING_CHECKPOINT_CLEAR_TILE_COL,
	SETTING_CHECKPOINT_CLEAR_TILE_ROW,
	SETTING_SKELETON_LEVEL,
	SETTING_SKELETON_ROOM,
	SETTING_SKELETON_TRIGGER_COLUMN_1,
	SETTING_SKELETON_TRIGGER_COLUMN_2,
	SETTING_SKELETON_COLUMN,
	SETTING_SKELETON_ROW,
	SETTING_SKELETON_REQUIRE_OPEN_LEVEL_DOOR,
	SETTING_SKELETON_SKILL,
	SETTING_SKELETON_REAPPEAR_ROOM,
	SETTING_SKELETON_REAPPEAR_X,
	SETTING_SKELETON_REAPPEAR_ROW,
	SETTING_SKELETON_REAPPEAR_DIR,
	SETTING_MIRROR_LEVEL,
	SETTING_MIRROR_ROOM,
	SETTING_MIRROR_COLUMN,
	SETTING_MIRROR_ROW,
	SETTING_MIRROR_TILE,
	SETTING_SHOW_MIRROR_IMAGE,
	SETTING_FALLING_EXIT_LEVEL,
	SETTING_FALLING_EXIT_ROOM,
	SETTING_FALLING_ENTRY_LEVEL,
	SETTING_FALLING_ENTRY_ROOM,
	SETTING_MOUSE_LEVEL,
	SETTING_MOUSE_ROOM,
	SETTING_MOUSE_DELAY,
	SETTING_MOUSE_OBJECT,
	SETTING_MOUSE_START_X,
	SETTING_LOOSE_TILES_LEVEL,
	SETTING_LOOSE_TILES_ROOM_1,
	SETTING_LOOSE_TILES_ROOM_2,
	SETTING_LOOSE_TILES_FIRST_TILE,
	SETTING_LOOSE_TILES_LAST_TILE,
	SETTING_JAFFAR_VICTORY_LEVEL,
	SETTING_JAFFAR_VICTORY_FLASH_TIME,
	SETTING_HIDE_LEVEL_NUMBER_FIRST_LEVEL,
	SETTING_LEVEL_13_LEVEL_NUMBER,
	SETTING_VICTORY_STOPS_TIME_LEVEL,
	SETTING_WIN_LEVEL,
	SETTING_WIN_ROOM,
	SETTING_LOOSE_FLOOR_DELAY,
	SETTING_BASE_SPEED,
	SETTING_FIGHT_SPEED,
	SETTING_CHOMPER_SPEED,
	SETTING_LEVEL_SETTINGS,
	SETTING_LEVEL_TYPE,
	SETTING_LEVEL_COLOR,
	SETTING_GUARD_TYPE,
	SETTING_GUARD_HP,
	SETTING_CUTSCENE,
	SETTING_ENTRY_POSE,
	SETTING_SEAMLESS_EXIT,
};

typedef struct setting_type {
	int index;
	int id;
	int previous, next;
	byte style;
	byte number_type;
	void* linked;
	void* required;
	int min, max; // for 'number'-style settings
	char text[64];
	char explanation[256];
	names_list_type* names_list;
} setting_type;

setting_type general_settings[] = {
		{.id = SETTING_SHOW_MENU_ON_PAUSE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_pause_menu,
				.text = "Enable pause menu",
				.explanation = "Show the in-game menu when you pause the game.\n"
						"If disabled, you can still bring up the menu by pressing Backspace."},
		{.id = SETTING_ENABLE_INFO_SCREEN, .style = SETTING_STYLE_TOGGLE, .linked = &enable_info_screen,
				.text = "Display info screen on launch",
				.explanation = "Display the SDLPoP information screen when the game starts."},
		{.id = SETTING_ENABLE_SOUND, .style = SETTING_STYLE_TOGGLE, .linked = &is_sound_on,
				.text = "Enable sound",
				.explanation = "Turn sound on or off."},
		{.id = SETTING_ENABLE_MUSIC, .style = SETTING_STYLE_TOGGLE, .linked = &enable_music,
				.text = "Enable music",
				.explanation = "Turn music on or off."},
		{.id = SETTING_ENABLE_CONTROLLER_RUMBLE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_controller_rumble,
				.text = "Enable controller rumble",
				.explanation = "If using a controller with a rumble motor, provide haptic feedback when the kid is hurt."},
		{.id = SETTING_JOYSTICK_THRESHOLD, .style = SETTING_STYLE_NUMBER, .number_type = SETTING_INT,
				.linked = &joystick_threshold, .min = 0, .max = INT16_MAX,
				.text = "Joystick threshold",
				.explanation = "Joystick 'dead zone' sensitivity threshold."},
		{.id = SETTING_JOYSTICK_ONLY_HORIZONTAL, .style = SETTING_STYLE_TOGGLE, .linked = &joystick_only_horizontal,
				.text = "Horizontal joystick movement only",
				.explanation = "Use joysticks for horizontal movement only, not all-directional. "
						"This may make the game easier to control for some controllers."},
		{.id = SETTING_RESET_ALL_SETTINGS, .style = SETTING_STYLE_TEXT_ONLY,
				.text = "Restore defaults...", .explanation = "Revert all settings to the default state."},
};

NAMES_LIST(scaling_type_setting_names, {"Sharp", "Fuzzy", "Blurry",});

int integer_scaling_possible =
#if SDL_VERSION_ATLEAST(2,0,5) // SDL_RenderSetIntegerScale
	1
#else
	0
#endif
;

setting_type visuals_settings[] = {
		{.id = SETTING_FULLSCREEN, .style = SETTING_STYLE_TOGGLE, .linked = &start_fullscreen,
				.text = "Start fullscreen",
				.explanation = "Start the game in fullscreen mode.\nYou can also toggle fullscreen by pressing Alt+Enter."},
		{.id = SETTING_USE_CORRECT_ASPECT_RATIO, .style = SETTING_STYLE_TOGGLE, .linked = &use_correct_aspect_ratio,
				.text = "Use 4:3 aspect ratio",
				.explanation = "Render the game in the originally intended 4:3 aspect ratio."
				               "\nNB. Works best using a high resolution."},
		{.id = SETTING_USE_INTEGER_SCALING, .style = SETTING_STYLE_TOGGLE, .linked = &use_integer_scaling,
				.required = &integer_scaling_possible,
				.text = "Use integer scaling",
				.explanation = "Enable pixel perfect scaling. That is, make all pixels the same size by forcing integer scale factors.\n"
						"Combining with 4:3 aspect ratio requires at least 1600x1200."
						"\nYou need to compile with SDL 2.0.5 or newer to enable this."},
		{.id = SETTING_SCALING_TYPE, .style = SETTING_STYLE_NUMBER, .number_type = SETTING_BYTE, .max = 2,
				.linked = &scaling_type, .names_list = &scaling_type_setting_names_list,
				.text = "Scaling method",
				.explanation = "Sharp - Use nearest neighbour resampling.\n"
						"Fuzzy - First upscale to double size, then use smooth scaling.\n"
						"Blurry - Use smooth scaling."},
#ifdef USE_FADE
		{.id = SETTING_ENABLE_FADE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_fade,
				.text = "Fading enabled",
				.explanation = "Turn fading on or off."},
#endif
#ifdef USE_FLASH
		{.id = SETTING_ENABLE_FLASH, .style = SETTING_STYLE_TOGGLE, .linked = &enable_flash,
				.text = "Flashing enabled",
				.explanation = "Turn flashing on or off."},
#endif
#ifdef USE_LIGHTING
		{.id = SETTING_ENABLE_LIGHTING, .style = SETTING_STYLE_TOGGLE, .linked = &enable_lighting,
				.text = "Torch shadows enabled",
				.explanation = "Darken those parts of the screen which are not near a torch."},
#endif
};

setting_type gameplay_settings[] = {
		{.id = SETTING_ENABLE_CHEATS, .style = SETTING_STYLE_TOGGLE, .linked = &cheats_enabled,
				.text = "Enable cheats",
				.explanation = "Turn cheats on or off."/*"\nAlso, display the CHEATS option on the pause menu."*/},
#ifdef USE_COPYPROT
		{.id = SETTING_ENABLE_COPYPROT, .style = SETTING_STYLE_TOGGLE, .linked = &enable_copyprot,
				.text = "Enable copy protection level",
				.explanation = "Enable or disable the potions (copy protection) level."},
#endif
#ifdef USE_QUICKSAVE
		{.id = SETTING_ENABLE_QUICKSAVE, .style = SETTING_STYLE_TOGGLE, .linked = &enable_quicksave,
				.text = "Enable quicksave",
				.explanation = "Enable quicksave/load feature.\nPress F6 to quicksave, F9 to quickload."},
		{.id = SETTING_ENABLE_QUICKSAVE_PENALTY, .style = SETTING_STYLE_TOGGLE, .linked = &enable_quicksave_penalty,
				.text = "Quicksave time penalty",
				.explanation = "Try to let time run out when quickloading (similar to dying).\n"
						"Actually, the 'remaining time' will still be restored, "
						"but a penalty (up to one minute) will be applied."},
#endif
#ifdef USE_REPLAY
		{.id = SETTING_ENABLE_REPLAY, .style = SETTING_STYLE_TOGGLE, .linked = &enable_replay,
				.text = "Enable replays",
				.explanation = "Enable recording/replay feature.\n"
						"Press Ctrl+Tab in-game to start recording.\n"
						"To stop, press Ctrl+Tab again."},
#endif
		{.id = SETTING_USE_FIXES_AND_ENHANCEMENTS, .style = SETTING_STYLE_TOGGLE, .linked = &use_fixes_and_enhancements,
				.text = "Enhanced mode (allow bug fixes)",
				.explanation = "Turn on game fixes and enhancements.\n"
						"Below, you can turn individual fixes/enhancements on or off.\n"
						"NOTE: Some fixes disable 'tricks' that depend on game quirks."},
		{.id = SETTING_ENABLE_CROUCH_AFTER_CLIMBING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.enable_crouch_after_climbing, .required = &use_fixes_and_enhancements,
				.text = "Enable crouching after climbing",
				.explanation = "Adds a way to crouch immediately after climbing up: press down and forward simultaneously. "
						"In the original game, this could not be done (pressing down always causes the kid to climb down)."},
		{.id = SETTING_ENABLE_FREEZE_TIME_DURING_END_MUSIC, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.enable_freeze_time_during_end_music, .required = &use_fixes_and_enhancements,
				.text = "Freeze time during level end music",
				.explanation = "Time runs out while the level ending music plays; however, the music can be skipped by disabling sound. "
						"This option stops time while the ending music is playing (so there is no need to disable sound)."},
		{.id = SETTING_ENABLE_REMEMBER_GUARD_HP, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.enable_remember_guard_hp, .required = &use_fixes_and_enhancements,
				.text = "Remember guard hitpoints",
				.explanation = "Enable guard hitpoints not resetting to their default (maximum) value when re-entering the room."},
		{.id = SETTING_enable_super_high_jump, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.enable_super_high_jump, .required = &use_fixes_and_enhancements,
				.text = "Enable super high jump",
				.explanation = "Prince in feather mode (after drinking a green potion) can jump 2 stories high."},
		{.id = SETTING_FIX_GATE_SOUNDS, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_gate_sounds, .required = &use_fixes_and_enhancements,
				.text = "Fix gate sounds bug",
				.explanation = "If a room is linked to itself on the left, the closing sounds of the gates in that room can't be heard."},
		{.id = SETTING_TWO_COLL_BUG, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_two_coll_bug, .required = &use_fixes_and_enhancements,
				.text = "Fix two collisions bug",
				.explanation = "An open gate or chomper may enable the Kid to go through walls. (Trick 7, 37, 62)"},
		{.id = SETTING_FIX_INFINITE_DOWN_BUG, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_infinite_down_bug, .required = &use_fixes_and_enhancements,
				.text = "Fix infinite down bug",
				.explanation = "If a room is linked to itself at the bottom, and the Kid's column has no floors, the game hangs."},
		{.id = SETTING_FIX_GATE_DRAWING_BUG, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_gate_drawing_bug, .required = &use_fixes_and_enhancements,
				.text = "Fix gate drawing bug",
				.explanation = "When a gate is under another gate, the top of the bottom gate is not visible."},
		{.id = SETTING_FIX_BIGPILLAR_CLIMB, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_bigpillar_climb, .required = &use_fixes_and_enhancements,
				.text = "Fix big pillar climbing bug",
				.explanation = "When climbing up to a floor with a big pillar top behind, turned right, Kid sees through floor."},
		{.id = SETTING_FIX_JUMP_DISTANCE_AT_EDGE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_jump_distance_at_edge, .required = &use_fixes_and_enhancements,
				.text = "Fix jump distance at edge",
				.explanation = "When climbing up two floors, turning around and jumping upward, the kid falls down. "
						"This fix makes the workaround of Trick 25 unnecessary."},
		{.id = SETTING_FIX_EDGE_DISTANCE_CHECK_WHEN_CLIMBING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_edge_distance_check_when_climbing, .required = &use_fixes_and_enhancements,
				.text = "Fix edge distance check when climbing",
				.explanation = "When climbing to a higher floor, the game unnecessarily checks how far away the edge below is. "
						"Sometimes you will \"teleport\" some distance when climbing from firm ground."},
		{.id = SETTING_FIX_PAINLESS_FALL_ON_GUARD, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_painless_fall_on_guard, .required = &use_fixes_and_enhancements,
				.text = "Fix painless fall on guard",
				.explanation = "Falling from a great height directly on top of guards does not hurt."},
		{.id = SETTING_FIX_WALL_BUMP_TRIGGERS_TILE_BELOW, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_wall_bump_triggers_tile_below, .required = &use_fixes_and_enhancements,
				.text = "Fix wall bump triggering tile below",
				.explanation = "Bumping against a wall may cause a loose floor below to drop, even though it has not been touched. (Trick 18, 34)"},
		{.id = SETTING_FIX_STAND_ON_THIN_AIR, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_stand_on_thin_air, .required = &use_fixes_and_enhancements,
				.text = "Fix standing on thin air",
				.explanation = "When pressing a loose tile, you can temporarily stand on thin air by standing up from crouching."},
		{.id = SETTING_FIX_PRESS_THROUGH_CLOSED_GATES, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_press_through_closed_gates, .required = &use_fixes_and_enhancements,
				.text = "Fix pressing through closed gates",
				.explanation = "Buttons directly to the right of gates can be pressed even though the gate is closed (Trick 1)"},
		{.id = SETTING_FIX_GRAB_FALLING_SPEED, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_grab_falling_speed, .required = &use_fixes_and_enhancements,
				.text = "Fix grab falling speed",
				.explanation = "By jumping and bumping into a wall, you can sometimes grab a ledge two stories down (which should not be possible)."},
		{.id = SETTING_FIX_SKELETON_CHOMPER_BLOOD, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_skeleton_chomper_blood, .required = &use_fixes_and_enhancements,
				.text = "Fix skeleton chomper blood",
				.explanation = "When chomped, skeletons cause the chomper to become bloody even though skeletons do not have blood."},
		{.id = SETTING_FIX_MOVE_AFTER_DRINK, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_move_after_drink, .required = &use_fixes_and_enhancements,
				.text = "Fix movement after drinking",
				.explanation = "Controls do not get released properly when drinking a potion, sometimes causing unintended movements."},
		{.id = SETTING_FIX_LOOSE_LEFT_OF_POTION, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_loose_left_of_potion, .required = &use_fixes_and_enhancements,
				.text = "Fix loose floor left of potion",
				.explanation = "A drawing bug occurs when a loose tile is placed to the left of a potion (or sword)."},
		{.id = SETTING_FIX_GUARD_FOLLOWING_THROUGH_CLOSED_GATES, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_guard_following_through_closed_gates, .required = &use_fixes_and_enhancements,
				.text = "Fix guards passing closed gates",
				.explanation = "Guards may \"follow\" the kid to the room on the left or right, even though there is a closed gate in between."},
		{.id = SETTING_FIX_SAFE_LANDING_ON_SPIKES, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_safe_landing_on_spikes, .required = &use_fixes_and_enhancements,
				.text = "Fix safe landing on spikes",
				.explanation = "When landing on the edge of a spikes tile, it is considered safe. (Trick 65)"},
		{.id = SETTING_FIX_GLIDE_THROUGH_WALL, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_glide_through_wall, .required = &use_fixes_and_enhancements,
				.text = "Fix gliding through walls",
				.explanation = "The kid may glide through walls after turning around while running (especially when weightless)."},
		{.id = SETTING_FIX_DROP_THROUGH_TAPESTRY, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_drop_through_tapestry, .required = &use_fixes_and_enhancements,
				.text = "Fix dropping through tapestries",
				.explanation = "The kid can drop down through a closed gate, when there is a tapestry (doortop) above the gate."},
		{.id = SETTING_FIX_LAND_AGAINST_GATE_OR_TAPESTRY, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_land_against_gate_or_tapestry, .required = &use_fixes_and_enhancements,
				.text = "Fix land against gate or tapestry",
				.explanation = "When dropping down and landing right in front of a wall, the entire landing animation should normally play. "
						"However, when falling against a closed gate or a tapestry(+floor) tile, the animation aborts."},
		{.id = SETTING_FIX_UNINTENDED_SWORD_STRIKE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_unintended_sword_strike, .required = &use_fixes_and_enhancements,
				.text = "Fix unintended sword strike",
				.explanation = "Sometimes, the kid may automatically strike immediately after drawing the sword. "
						"This especially happens when dropping down from a higher floor and then turning towards the opponent."},
		{.id = SETTING_FIX_RETREAT_WITHOUT_LEAVING_ROOM, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_retreat_without_leaving_room, .required = &use_fixes_and_enhancements,
				.text = "Fix retreat without leaving room",
				.explanation = "By repeatedly pressing 'back' in a swordfight, you can retreat out of a room without the room changing. (Trick 35)"},
		{.id = SETTING_FIX_RUNNING_JUMP_THROUGH_TAPESTRY, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_running_jump_through_tapestry, .required = &use_fixes_and_enhancements,
				.text = "Fix running jumps through tapestries",
				.explanation = "The kid can jump through a tapestry with a running jump to the left, if there is a floor above it."},
		{.id = SETTING_FIX_PUSH_GUARD_INTO_WALL, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_push_guard_into_wall, .required = &use_fixes_and_enhancements,
				.text = "Fix pushing guards into walls",
				.explanation = "Guards can be pushed into walls, because the game does not correctly check for walls located behind a guard."},
		{.id = SETTING_FIX_JUMP_THROUGH_WALL_ABOVE_GATE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_jump_through_wall_above_gate, .required = &use_fixes_and_enhancements,
				.text = "Fix jump through wall above gate",
				.explanation = "By doing a running jump into a wall, you can fall behind a closed gate two floors down. (e.g. skip in Level 7)"},
		{.id = SETTING_FIX_CHOMPERS_NOT_STARTING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_chompers_not_starting, .required = &use_fixes_and_enhancements,
				.text = "Fix chompers not starting",
				.explanation = "If you grab a ledge that is one or more floors down, the chompers on that row will not start."},
		{.id = SETTING_FIX_FEATHER_INTERRUPTED_BY_LEVELDOOR, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_feather_interrupted_by_leveldoor, .required = &use_fixes_and_enhancements,
				.text = "Fix leveldoor interrupting feather fall",
				.explanation = "As soon as a level door has completely opened, the feather fall effect is interrupted because the sound stops."},
		{.id = SETTING_FIX_OFFSCREEN_GUARDS_DISAPPEARING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_offscreen_guards_disappearing, .required = &use_fixes_and_enhancements,
				.text = "Fix offscreen guards disappearing",
				.explanation = "Guards will often not reappear in another room if they have been pushed (partly or entirely) offscreen."},
		{.id = SETTING_FIX_MOVE_AFTER_SHEATHE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_move_after_sheathe, .required = &use_fixes_and_enhancements,
				.text = "Fix movement after sheathing",
				.explanation = "While putting the sword away, if you press forward and down, and then release down, the kid will still duck."},
		{.id = SETTING_FIX_HIDDEN_FLOORS_DURING_FLASHING, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_hidden_floors_during_flashing, .required = &use_fixes_and_enhancements,
				.text = "Fix hidden floors during flashing",
				.explanation = "After uniting with the shadow in level 12, the hidden floors will not appear until after the flashing stops."},
		{.id = SETTING_FIX_HANG_ON_TELEPORT, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_hang_on_teleport, .required = &use_fixes_and_enhancements,
				.text = "Fix hang on teleport bug",
				.explanation = "By jumping towards one of the bottom corners of the room and grabbing a ledge, you can teleport to the room above."},
		{.id = SETTING_FIX_EXIT_DOOR, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_exit_door, .required = &use_fixes_and_enhancements,
				.text = "Fix exit doors",
				.explanation = "You can enter closed exit doors after you met the shadow or Jaffar died, or after you opened one of multiple exits."},
		{.id = SETTING_FIX_QUICKSAVE_DURING_FEATHER, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_quicksave_during_feather, .required = &use_fixes_and_enhancements,
				.text = "Fix quick save in feather mode",
				.explanation = "You cannot save game while floating in feather mode."},
		{.id = SETTING_FIX_CAPED_PRINCE_SLIDING_THROUGH_GATE, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_caped_prince_sliding_through_gate, .required = &use_fixes_and_enhancements,
				.text = "Fix sliding through closed gate",
				.explanation = "If you are using the caped prince graphics, and crouch with your back towards a closed gate on the left edge on the room, then the prince will slide through the gate."},
		{.id = SETTING_FIX_DOORTOP_DISABLING_GUARD, .style = SETTING_STYLE_TOGGLE,
				.linked = &fixes_saved.fix_doortop_disabling_guard, .required = &use_fixes_and_enhancements,
				.text = "Fix door top disabling guard",
				.explanation = "Guards become inactive if they are standing on a door top (with floor), or if the prince is standing on a door top."},
};

NAMES_LIST(tile_type_setting_names, {
		"Empty", "Floor", "Spikes", "Pillar", "Gate",                                                    // 0..4
		"Stuck button", "Closer button", "Tapestry/floor", "Big pillar: bottom", "Big pillar: top",      // 5..9
		"Potion", "Loose floor", "Tapestry", "Mirror", "Floor/debris",                                   // 10..14
		"Raise button", "Level door: left", "Level door: right", "Chomper", "Torch",                     // 15..19
		"Wall", "Skeleton", "Sword", "Balcony: left", "Balcony: right",                                  // 20..24
		"Lattice: pillar", "Lattice: down", "Lattice: small", "Lattice: left", "Lattice: right",         // 25..29
		"Torch/debris", "Tile 31 (unused)" // 30
});
NAMES_LIST(row_setting_names, {"Top", "Middle", "Bottom"});
KEY_VALUE_LIST(direction_setting_names, {{"Left", dir_FF_left}, {"Right", dir_0_right}});
extern names_list_type never_is_16_list;

setting_type mods_settings[] = {
		{.id = SETTING_USE_CUSTOM_OPTIONS, .style = SETTING_STYLE_TOGGLE, .linked = &use_custom_options,
				.text = "Use customization options",
				.explanation = "Turn customization options on or off.\n(default = OFF)"},
		{.id = SETTING_LEVEL_SETTINGS, .style = SETTING_STYLE_TEXT_ONLY, .required = &use_custom_options,
				.text = "Customize level...",
				.explanation = "Change level-specific options (such as level type, guard type, number of guard hitpoints)."},
		{.id = SETTING_START_MINUTES_LEFT, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.start_minutes_left, .number_type = SETTING_SHORT, .min = -1, .max = INT16_MAX,
				.text = "Starting minutes left",
				.explanation = "Starting minutes left. (default = 60)\n"
						"To disable the time limit completely, set this to -1."},
		{.id = SETTING_START_TICKS_LEFT, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.start_ticks_left, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Starting seconds left",
				.explanation = "Starting number of seconds left in the first minute.\n(default = 59.92)"},
		{.id = SETTING_START_HITP, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.start_hitp, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Starting hitpoints",
				.explanation = "Starting hitpoints. (default = 3)"},
		{.id = SETTING_MAX_HITP_ALLOWED, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.max_hitp_allowed, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Max hitpoints allowed",
				.explanation = "Maximum number of hitpoints you can get. (default = 10)"},
		{.id = SETTING_SAVING_ALLOWED_FIRST_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.saving_allowed_first_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Saving allowed: first level",
				.explanation = "First level where you can save the game. (default = 3)"},
		{.id = SETTING_SAVING_ALLOWED_LAST_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.saving_allowed_last_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Saving allowed: last level",
				.explanation = "Last level where you can save the game. (default = 13)"},
		{.id = SETTING_START_UPSIDE_DOWN, .style = SETTING_STYLE_TOGGLE, .required = &use_custom_options,
				.linked = &custom_saved.start_upside_down,
				.text = "Start with the screen flipped",
				.explanation = "Start the game with the screen flipped upside down, similar to Shift+I (default = OFF)"},
		{.id = SETTING_START_IN_BLIND_MODE, .style = SETTING_STYLE_TOGGLE, .required = &use_custom_options,
				.linked = &custom_saved.start_in_blind_mode,
				.text = "Start in blind mode",
				.explanation = "Start in blind mode, similar to Shift+B (default = OFF)"},
		{.id = SETTING_COPYPROT_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.copyprot_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Copy protection before level",
				.explanation = "The potions level will appear before this level. (default = 2)"},
		{.id = SETTING_DRAWN_TILE_TOP_LEVEL_EDGE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.names_list = &tile_type_setting_names_list,
				.linked = &custom_saved.drawn_tile_top_level_edge, .number_type = SETTING_BYTE, .max = 31,
				.text = "Drawn tile: top level edge",
				.explanation = "Tile drawn at the top of the room if there is no room that way. (default = floor)"},
		{.id = SETTING_DRAWN_TILE_LEFT_LEVEL_EDGE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.names_list = &tile_type_setting_names_list,
				.linked = &custom_saved.drawn_tile_left_level_edge, .number_type = SETTING_BYTE, .max = 31,
				.text = "Drawn tile: left level edge",
				.explanation = "Tile drawn at the left of the room if there is no room that way. (default = wall)"},
		{.id = SETTING_LEVEL_EDGE_HIT_TILE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.names_list = &tile_type_setting_names_list,
				.linked = &custom_saved.level_edge_hit_tile, .number_type = SETTING_BYTE, .max = 31,
				.text = "Level edge hit tile",
				.explanation = "Tile behavior at the top or left of the room if there is no room that way (default = wall)"},
		{.id = SETTING_ALLOW_TRIGGERING_ANY_TILE, .style = SETTING_STYLE_TOGGLE, .required = &use_custom_options,
				.linked = &custom_saved.allow_triggering_any_tile,
				.text = "Allow triggering any tile",
				.explanation = "Enable triggering any tile. For example a button could make loose floors fall, or start a stuck chomper. (default = OFF)"},
		{.id = SETTING_ENABLE_WDA_IN_PALACE, .style = SETTING_STYLE_TOGGLE, .required = &use_custom_options,
				.linked = &custom_saved.enable_wda_in_palace,
				.text = "Enable WDA in palace",
				.explanation = "Enable the dungeon wall drawing algorithm in the palace."
						"\nN.B. Use with a modified VPALACE.DAT that provides dungeon-like wall graphics! (default = OFF)"},
		{.id = SETTING_FIRST_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.first_level, .number_type = SETTING_WORD, .max = 15,
				.text = "First level",
				.explanation = "Level that will be loaded when starting a new game."
						"\n(default = 1)"},
		{.id = SETTING_SKIP_TITLE, .style = SETTING_STYLE_TOGGLE, .required = &use_custom_options,
				.linked = &custom_saved.skip_title,
				.text = "Skip title sequence",
				.explanation = "Always skip the title sequence: the first level will be loaded immediately."
						"\n(default = OFF)"},
		{.id = SETTING_SHIFT_L_ALLOWED_UNTIL_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.shift_L_allowed_until_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Shift+L allowed until level",
				.explanation = "First level where level skipping with Shift+L is denied in non-cheat mode.\n"
						"(default = 4)"},
		{.id = SETTING_SHIFT_L_REDUCED_MINUTES, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.shift_L_reduced_minutes, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Minutes left after Shift+L used",
				.explanation = "Number of minutes left after Shift+L is used in non-cheat mode.\n"
						"(default = 15)"},
		{.id = SETTING_SHIFT_L_REDUCED_TICKS, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.shift_L_reduced_ticks, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Seconds left after Shift+L used",
				.explanation = "Number of seconds left after Shift+L is used in non-cheat mode.\n(default = 59.92)"},
		{.id = SETTING_DEMO_HITP, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.demo_hitp, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Demo level hitpoints",
				.explanation = "Hitpoints the kid has on the demo level.\n(default = 4)"},
		{.id = SETTING_DEMO_END_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.demo_end_room, .number_type = SETTING_WORD, .min = 1, .max = 24,
				.text = "Demo level ending room",
				.explanation = "Demo level ending room.\n(default = 24)"},
		{.id = SETTING_INTRO_MUSIC_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.intro_music_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Level with intro music",
				.explanation = "Level where the presentation music is played when the kid crouches down. (default = 1)\n"
						"Note: only works if this level is the starting level."},
		{.id = SETTING_HAVE_SWORD_FROM_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.have_sword_from_level, .number_type = SETTING_WORD, .min = 1, .max = 16, .names_list = &never_is_16_list,
				.text = "Have sword from level",
				.explanation = "First level (except the demo level) where kid has the sword.\n(default = 2)\n"},
		{.id = SETTING_CHECKPOINT_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.checkpoint_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Checkpoint level",
				.explanation = "Level where there is a checkpoint. (default = 3)\n"
						"The checkpoint is triggered when leaving room 7 to the left."},
		{.id = SETTING_CHECKPOINT_RESPAWN_DIR, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.checkpoint_respawn_dir, .number_type = SETTING_SBYTE, .min = -1, .max = 0, .names_list = &direction_setting_names_list,
				.text = "Checkpoint respawn direction",
				.explanation = "Respawn direction after triggering the checkpoint.\n(default = left)"},
		{.id = SETTING_CHECKPOINT_RESPAWN_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.checkpoint_respawn_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Checkpoint respawn room",
				.explanation = "Room where you respawn after triggering the checkpoint.\n(default = 2)"},
		{.id = SETTING_CHECKPOINT_RESPAWN_TILEPOS, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.checkpoint_respawn_tilepos, .number_type = SETTING_BYTE, .max = 29,
				.text = "Checkpoint respawn tile position",
				.explanation = "Tile position (0 to 29) where you respawn after triggering the checkpoint.\n(default = 6)"},
		{.id = SETTING_CHECKPOINT_CLEAR_TILE_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.checkpoint_clear_tile_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Checkpoint clear tile room",
				.explanation = "Room where a tile is cleared after respawning at the checkpoint location.\n(default = 7)"},
		{.id = SETTING_CHECKPOINT_CLEAR_TILE_COL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.checkpoint_clear_tile_col, .number_type = SETTING_BYTE, .max = 9,
				.text = "Checkpoint clear tile column",
				.explanation = "Location (column/row) of the cleared tile after respawning at the checkpoint location.\n(default: column = 4, row = top)"},
		{.id = SETTING_CHECKPOINT_CLEAR_TILE_ROW, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.checkpoint_clear_tile_row, .number_type = SETTING_BYTE, .max = 2, .names_list = &row_setting_names_list,
				.text = "Checkpoint clear tile row",
				.explanation = "Location (column/row) of the cleared tile after respawning at the checkpoint location.\n(default: column = 4, row = top)"},
		{.id = SETTING_SKELETON_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Skeleton awakes level",
				.explanation = "Level and room where a skeleton can come alive.\n(default: level = 3, room = 1)"},
		{.id = SETTING_SKELETON_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Skeleton awakes room",
				.explanation = "Level and room where a skeleton can come alive.\n(default: level = 3, room = 1)"},
		{.id = SETTING_SKELETON_TRIGGER_COLUMN_1, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_trigger_column_1, .number_type = SETTING_BYTE, .max = 9,
				.text = "Skeleton trigger column (1)",
				.explanation = "The skeleton will wake up if the kid is on one of these two columns.\n(defaults = 2,3)"},
		{.id = SETTING_SKELETON_TRIGGER_COLUMN_2, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_trigger_column_2, .number_type = SETTING_BYTE, .max = 9,
				.text = "Skeleton trigger column (2)",
				.explanation = "The skeleton will wake up if the kid is on one of these two columns.\n(defaults = 2,3)"},
		{.id = SETTING_SKELETON_COLUMN, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_column, .number_type = SETTING_BYTE, .max = 9,
				.text = "Skeleton tile column",
				.explanation = "Location (column/row) of the skeleton tile that will awaken.\n(default: column = 5, row = middle)"},
		{.id = SETTING_SKELETON_ROW, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_row, .number_type = SETTING_BYTE, .max = 2, .names_list = &row_setting_names_list,
				.text = "Skeleton tile row",
				.explanation = "Location (column/row) of the skeleton tile that will awaken.\n(default: column = 5, row = middle)"},
		{.id = SETTING_SKELETON_REQUIRE_OPEN_LEVEL_DOOR, .style = SETTING_STYLE_TOGGLE, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_require_open_level_door,
				.text = "Skeleton requires level door",
				.explanation = "Whether the level door must first be opened before the skeleton awakes.\n(default = true)"},
		{.id = SETTING_SKELETON_SKILL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_skill, .number_type = SETTING_BYTE, .max = 15,
				.text = "Skeleton skill",
				.explanation = "Skill of the awoken skeleton.\n(default = 2)"},
		{.id = SETTING_SKELETON_REAPPEAR_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_reappear_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Skeleton reappear room",
				.explanation = "If the skeleton falls into this room, it will reappear there.\n(default = 3)"},
		{.id = SETTING_SKELETON_REAPPEAR_X, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_reappear_x, .number_type = SETTING_BYTE, .max = 255,
				.text = "Skeleton reappear X coordinate",
				.explanation = "Horizontal coordinate where the skeleton reappears.\n(default = 133)\n"
						"(58 = left edge of the room, 198 = right edge)"},
		{.id = SETTING_SKELETON_REAPPEAR_ROW, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_reappear_row, .number_type = SETTING_BYTE, .max = 2, .names_list = &row_setting_names_list,
				.text = "Skeleton reappear row",
				.explanation = "Row on which the skeleton reappears.\n(default = middle)"},
		{.id = SETTING_SKELETON_REAPPEAR_DIR, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.skeleton_reappear_dir, .number_type = SETTING_SBYTE, .min = -1, .max = 0, .names_list = &direction_setting_names_list,
				.text = "Skeleton reappear direction",
				.explanation = "Direction the skeleton is facing when it reappears.\n(default = right)"},
		{.id = SETTING_MIRROR_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mirror_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Mirror level",
				.explanation = "Level and room where the mirror appears.\n(default: level = 4, room = 4)"},
		{.id = SETTING_MIRROR_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mirror_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Mirror room",
				.explanation = "Level and room where the mirror appears.\n(default: level = 4, room = 4)"},
		{.id = SETTING_MIRROR_COLUMN, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mirror_column, .number_type = SETTING_BYTE, .max = 9,
				.text = "Mirror column",
				.explanation = "Location (column/row) of the tile where the mirror appears.\n(default: column = 4, row = top)"},
		{.id = SETTING_MIRROR_ROW, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mirror_row, .number_type = SETTING_BYTE, .max = 2, .names_list = &row_setting_names_list,
				.text = "Mirror row",
				.explanation = "Location (column/row) of the tile where the mirror appears.\n(default: column = 4, row = top)"},
		{.id = SETTING_MIRROR_TILE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mirror_tile, .number_type = SETTING_BYTE, .max = 31, .names_list = &tile_type_setting_names_list,
				.text = "Mirror tile",
				.explanation = "Tile type that appears when the mirror should appear.\n(default = mirror)"},
		{.id = SETTING_SHOW_MIRROR_IMAGE, .style = SETTING_STYLE_TOGGLE, .required = &use_custom_options,
				.linked = &custom_saved.show_mirror_image, .number_type = SETTING_BYTE,
				.text = "Show mirror image",
				.explanation = "Show the kid's mirror image in the mirror.\n(default = true)"},
		{.id = SETTING_FALLING_EXIT_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.falling_exit_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Falling exit level",
				.explanation = "Level where the kid can progress to the next level by falling off a specific room.\n(default = 6)"},
		{.id = SETTING_FALLING_EXIT_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.falling_exit_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Falling exit room",
				.explanation = "Room where the kid can progress to the next level by falling down.\n(default = 1)"},
		{.id = SETTING_FALLING_ENTRY_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.falling_entry_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Falling entry level",
				.explanation = "If the kid starts in this level in this room, the starting room will not be shown,\n"
						"but the room below instead, to allow for a falling entry. (default: level = 7, room = 17)"},
		{.id = SETTING_FALLING_ENTRY_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.falling_entry_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Falling entry room",
				.explanation = "If the kid starts in this level in this room, the starting room will not be shown,\n"
						"but the room below instead, to allow for a falling entry. (default: level = 7, room = 17)"},
		{.id = SETTING_MOUSE_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mouse_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Mouse level",
				.explanation = "Level where the mouse appears.\n(default = 8)"},
		{.id = SETTING_MOUSE_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mouse_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Mouse room",
				.explanation = "Room where the mouse appears.\n(default = 16)"},
		{.id = SETTING_MOUSE_DELAY, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mouse_delay, .number_type = SETTING_WORD, .max = UINT16_MAX,
				.text = "Mouse delay",
				.explanation = "Number of seconds to wait before the mouse appears.\n(default = 12.5)"},
		{.id = SETTING_MOUSE_OBJECT, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mouse_object, .number_type = SETTING_BYTE, .max = 255,
				.text = "Mouse object",
				.explanation = "Mouse object type. (default = 24)\n"
						"Be careful: a value not 24 will change the mouse for the kid."},
		{.id = SETTING_MOUSE_START_X, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.mouse_start_x, .number_type = SETTING_BYTE, .max = 255,
				.text = "Mouse start X coordinate",
				.explanation = "Horizontal starting coordinate of the mouse.\n(default = 200)"},
		{.id = SETTING_LOOSE_TILES_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.loose_tiles_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Loose tiles level",
				.explanation = "Level where loose floor tiles will fall down.\n(default = 13)"},
		{.id = SETTING_LOOSE_TILES_ROOM_1, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.loose_tiles_room_1, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Loose tiles room (1)",
				.explanation = "Rooms where visible loose floor tiles will fall down.\n(default = 23, 16)"},
		{.id = SETTING_LOOSE_TILES_ROOM_2, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.loose_tiles_room_2, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Loose tiles room (2)",
				.explanation = "Rooms where visible loose floor tiles will fall down.\n(default = 23, 16)"},
		{.id = SETTING_LOOSE_TILES_FIRST_TILE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.loose_tiles_first_tile, .number_type = SETTING_BYTE, .max = 29,
				.text = "Loose tiles first tile",
				.explanation = "Range of loose floor tile positions that will be pressed.\n(default = 22 to 27)"},
		{.id = SETTING_LOOSE_TILES_LAST_TILE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.loose_tiles_last_tile, .number_type = SETTING_BYTE, .max = 29,
				.text = "Loose tiles last tile",
				.explanation = "Range of loose floor tile positions that will be pressed.\n(default = 22 to 27)"},
		{.id = SETTING_JAFFAR_VICTORY_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.jaffar_victory_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Jaffar victory level",
				.explanation = "Killing the guard in this level causes the screen to flash, and event 0 to be triggered upon leaving the room.\n(default = 13)"},
		{.id = SETTING_JAFFAR_VICTORY_FLASH_TIME, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.jaffar_victory_flash_time, .number_type = SETTING_BYTE, .max = UINT16_MAX,
				.text = "Jaffar victory flash time",
				.explanation = "How long the screen will flash after killing Jaffar.\n(default = 18)"},
		{.id = SETTING_HIDE_LEVEL_NUMBER_FIRST_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.hide_level_number_from_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Hide level number from level",
				.explanation = "First level where the level number will not be displayed.\n(default = 14)"},
		{.id = SETTING_LEVEL_13_LEVEL_NUMBER, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.level_13_level_number, .number_type = SETTING_BYTE, .max = UINT16_MAX,
				.text = "Level 13 displayed level number",
				.explanation = "Level number displayed on level 13.\n(default = 12)"},
		{.id = SETTING_VICTORY_STOPS_TIME_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.victory_stops_time_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Victory stops time level",
				.explanation = "Level where Jaffar's death stops time.\n(default = 13)"},
		{.id = SETTING_WIN_LEVEL, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.win_level, .number_type = SETTING_WORD, .max = 16, .names_list = &never_is_16_list,
				.text = "Level where you can win",
				.explanation = "Level and room where you can win the game.\n(default: level = 14, room = 5)"},
		{.id = SETTING_WIN_ROOM, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.win_room, .number_type = SETTING_BYTE, .min = 1, .max = 24,
				.text = "Room where you can win",
				.explanation = "Level and room where you can win the game.\n(default: level = 14, room = 5)"},
		{.id = SETTING_LOOSE_FLOOR_DELAY, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.loose_floor_delay, .number_type = SETTING_BYTE, .min = 0, .max = 127,
				.text = "Loose floor delay",
				.explanation = "Number of seconds to wait before a loose floor falls.\n(default = 0.92)"},
		{.id = SETTING_BASE_SPEED, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.base_speed, .number_type = SETTING_BYTE, .min = 1, .max = 127,
				.text = "Base speed",
				.explanation = "Game speed when not fighting (delay between frames in 1/60 seconds). Smaller is faster.\n(default = 5)"},
		{.id = SETTING_FIGHT_SPEED, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.fight_speed, .number_type = SETTING_BYTE, .min = 1, .max = 127,
				.text = "Fight speed",
				.explanation = "Game speed when fighting (delay between frames in 1/60 seconds). Smaller is faster.\n(default = 6)"},
		{.id = SETTING_CHOMPER_SPEED, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = &custom_saved.chomper_speed, .number_type = SETTING_BYTE, .min = 0, .max = 127,
				.text = "Chomper speed",
				.explanation = "Chomper speed (length of the animation cycle in frames). Smaller is faster.\n(default = 15)"},
};

NAMES_LIST(level_type_setting_names, { "Dungeon", "Palace", });
KEY_VALUE_LIST(guard_type_setting_names, {{"None", -1}, {"Normal", 0}, {"Fat", 1}, {"Skeleton", 2}, {"Vizier", 3}, {"Shadow", 4}});
NAMES_LIST(entry_pose_setting_names, {"Turning", "Falling", "Running"});
KEY_VALUE_LIST(off_setting_name, {{"Off", -1}}); // used for the seamless exit setting

setting_type level_settings[] = {
		{.id = SETTING_LEVEL_SETTINGS, .style = SETTING_STYLE_TEXT_ONLY, .required = &use_custom_options,
				.text = "Customize another level...",
				.explanation = "Select another level to customize."},
		{.id = SETTING_LEVEL_TYPE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.names_list = &level_type_setting_names_list,
				.linked = NULL /* depends on which level */, .number_type = SETTING_BYTE, .max = 1,
				.text = "Level type",
				.explanation = "Which environment is used in this level.\n"
						"(either dungeon or palace)"},
		{.id = SETTING_LEVEL_COLOR, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = NULL, .number_type = SETTING_WORD, .max = 4,
				.text = "Level color palette",
				.explanation = "0: colors from VDUNGEON.DAT/VPALACE.DAT\n>0: colors from PRINCE.DAT.\n"
						"You need a PRINCE.DAT from PoP 1.3 or 1.4 for this."},
		{.id = SETTING_GUARD_TYPE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.names_list = &guard_type_setting_names_list,
				.linked = NULL, .number_type = SETTING_SHORT, .min = -1, .max = 4,
				.text = "Guard type",
				.explanation = "Guard type used in this level (normal, fat, skeleton, vizier, or shadow)."},
		{.id = SETTING_GUARD_HP, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = NULL, .number_type = SETTING_BYTE, .max = UINT8_MAX,
				.text = "Guard hitpoints",
				.explanation = "Number of hitpoints guards have in this level."},
		{.id = SETTING_CUTSCENE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = NULL, .number_type = SETTING_BYTE, .max = 15,
				.text = "Cutscene before level",
				.explanation = "Cutscene that plays between the previous level and this level.\n"
						"0: none, 2 or 6: standing, 4: lying down, 8: mouse leaves,\n"
						"9: mouse returns, 12: standing or turn around"},
		{.id = SETTING_ENTRY_POSE, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = NULL, .number_type = SETTING_BYTE, .max = 2, .names_list = &entry_pose_setting_names_list,
				.text = "Entry pose",
				.explanation = "The pose the kid has when the level starts.\n"},
		{.id = SETTING_SEAMLESS_EXIT, .style = SETTING_STYLE_NUMBER, .required = &use_custom_options,
				.linked = NULL, .number_type = SETTING_SBYTE, .min = -1, .max = 24, .names_list = &off_setting_name_list,
				.text = "Seamless exit",
				.explanation = "Entering this room moves the kid to the next level.\n"
						"Set to -1 to disable."},
};

typedef struct settings_area_type {
	setting_type* settings;
	int setting_count;
} settings_area_type;

settings_area_type general_settings_area = { .settings = general_settings, .setting_count = COUNT(general_settings)};
settings_area_type gameplay_settings_area = { .settings = gameplay_settings, .setting_count = COUNT(gameplay_settings)};
settings_area_type visuals_settings_area = { .settings = visuals_settings, .setting_count = COUNT(visuals_settings)};
settings_area_type mods_settings_area = { .settings = mods_settings, .setting_count = COUNT(mods_settings)};
settings_area_type level_settings_area = { .settings = level_settings, .setting_count = COUNT(level_settings)};

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
		case SETTINGS_MENU_LEVEL_CUSTOMIZATION:
			return &level_settings_area;
	}
}

void init_pause_menu_items(pause_menu_item_type* first_item, int item_count) {
	if (item_count > 0) {
		for (int i = 0; i < item_count; ++i) {
			pause_menu_item_type* item = first_item + i;
			item->previous = (first_item + MAX(0, i-1));
			item->next = (first_item + MIN(item_count-1, i+1));
		}
		pause_menu_item_type* last_item = first_item + (item_count-1);
		first_item->previous = last_item;
		last_item->next = first_item;
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
	init_settings_list(level_settings, COUNT(level_settings));
}

bool is_mouse_over_rect(rect_type* rect) {
	return (mouse_x >= rect->left && mouse_x < rect->right && mouse_y >= rect->top && mouse_y < rect->bottom);
}

// Maps the cursor position into a coordinate between (0,0) and (320,200) and sets mouse_x, mouse_y and mouse_moved.
void read_mouse_state() {
	float scale_x, scale_y;
	SDL_RenderGetScale(renderer_, &scale_x, &scale_y);
	int logical_width, logical_height;
	SDL_RenderGetLogicalSize(renderer_, &logical_width, &logical_height);
	int logical_scale_x = logical_width / 320; // These may be higher than 1, if 4:3 aspect ratio scaling is enabled.
	int logical_scale_y = logical_height / 200;
	scale_x *= logical_scale_x;
	scale_y *= logical_scale_y;
	if (!(scale_x > 0 && scale_y > 0 && logical_scale_x > 0 && logical_scale_y > 0)) return;
	SDL_Rect viewport;
	SDL_RenderGetViewport(renderer_, &viewport); // Get the width/height of the 'black bars' around the rendering area.
	viewport.x /= logical_scale_x;
	viewport.y /= logical_scale_y;
	int last_mouse_x = mouse_x;
	int last_mouse_y = mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	mouse_x = (int) ((float)mouse_x/scale_x - viewport.x + 0.5f);
	mouse_y = (int) ((float)mouse_y/scale_y - viewport.y + 0.5f);
	mouse_moved = (last_mouse_x != mouse_x || last_mouse_y != mouse_y);
}

rect_type explanation_rect = {170, 20, 200, 300};
int highlighted_setting_id = SETTING_ENABLE_INFO_SCREEN;
int controlled_area = 0; // Whether the focus is on the left (0) or right (1) part of the screen in the Settings menu.
int next_setting_id = 0; // For navigating up/down.
int previous_setting_id = 0;
bool at_scroll_up_boundary; // When navigating up using keyboard/controller, whether we also need to scroll up
bool at_scroll_down_boundary; // When navigating down using keyboard/controller, whether we also need to scroll down

void play_menu_sound(int sound_id) {
	play_sound(sound_id);
	play_next_sound();
}

void enter_settings_subsection(int settings_menu_id) {
	settings_area_type* settings_area = get_settings_area(settings_menu_id);
	if (active_settings_subsection != settings_menu_id) {
		highlighted_setting_id = settings_area->settings[0].id;
	}
	active_settings_subsection = settings_menu_id;
	highlighted_settings_subsection = settings_menu_id;
	if (!mouse_clicked) hovering_pause_menu_item = 0;
	controlled_area = 1;
	scroll_position = 0;

	// Special case: for the level customization submenu, the linked variables should depend on menu_current_level.
	// So we need to initialize them now.
	if (settings_menu_id == SETTINGS_MENU_LEVEL_CUSTOMIZATION) {
		for (int i = 0; i < settings_area->setting_count; ++i) {
			setting_type* setting = &settings_area->settings[i];
			switch(setting->id) {
				default: break;
				case SETTING_LEVEL_TYPE:
					setting->linked = &custom_saved.tbl_level_type[menu_current_level];
					break;
				case SETTING_LEVEL_COLOR:
					setting->linked = &custom_saved.tbl_level_color[menu_current_level];
					break;
				case SETTING_GUARD_TYPE:
					setting->linked = &custom_saved.tbl_guard_type[menu_current_level];
					break;
				case SETTING_GUARD_HP:
					setting->linked = &custom_saved.tbl_guard_hp[menu_current_level];
					break;
				case SETTING_CUTSCENE:
					setting->linked = &custom_saved.tbl_cutscenes_by_index[menu_current_level];
					break;
				case SETTING_ENTRY_POSE:
					setting->linked = &custom_saved.tbl_entry_pose[menu_current_level];
					break;
				case SETTING_SEAMLESS_EXIT:
					setting->linked = &custom_saved.tbl_seamless_exit[menu_current_level];
					break;
			}
		}
	}
}

void leave_settings_subsection() {
	if (active_settings_subsection == SETTINGS_MENU_LEVEL_CUSTOMIZATION) {
		enter_settings_subsection(SETTINGS_MENU_MODS);
	} else {
		// Go back to the top level of the settings menu.
		controlled_area = 0;
		hovering_pause_menu_item = active_settings_subsection;
		active_settings_subsection = 0;
		highlighted_settings_subsection = 0;
	}
}

void reset_paused_menu() {
	drawn_menu = 0;
	controlled_area = 0;
	hovering_pause_menu_item = PAUSE_MENU_RESUME;
}

void pause_menu_clicked(pause_menu_item_type* item) {
	//printf("Clicked option %s\n", item->text);
	play_menu_sound(sound_22_loose_shake_3);
	switch(item->id) {
		default: break;
		case PAUSE_MENU_RESUME:
			need_close_menu = true;
			break;
		case PAUSE_MENU_SAVE_GAME:
			// TODO: Manual save games?
			if (Kid.alive < 0) need_quick_save = 1;
			need_close_menu = true;
			break;
		case PAUSE_MENU_LOAD_GAME:
			// TODO: Manual save games?
			need_quick_load = 1;
			need_close_menu = true;
			stop_sounds();
			break;
		case PAUSE_MENU_RESTART_LEVEL:
			last_key_scancode = SDL_SCANCODE_A | WITH_CTRL;
			break;
		case PAUSE_MENU_SETTINGS:
			drawn_menu = 1;
			hovering_pause_menu_item = SETTINGS_MENU_GENERAL;
			highlighted_settings_subsection = SETTINGS_MENU_GENERAL;
			active_settings_subsection = 0;
			controlled_area = 0;
			break;
		case PAUSE_MENU_RESTART_GAME:
			last_key_scancode = SDL_SCANCODE_R | WITH_CTRL;
			break;
		case PAUSE_MENU_QUIT_GAME:
			current_dialog_box = DIALOG_CONFIRM_QUIT;
			current_dialog_text = "Quit SDLPoP?";
			break;
		case SETTINGS_MENU_GENERAL:
		case SETTINGS_MENU_GAMEPLAY:
		case SETTINGS_MENU_VISUALS:
		case SETTINGS_MENU_MODS:
			enter_settings_subsection(item->id);
			break;
		case SETTINGS_MENU_BACK:
			reset_paused_menu();
			break;
	}
	clear_menu_controls(); // prevent "click-through" because the screen changes
}

void draw_pause_menu_item(pause_menu_item_type* item, rect_type* parent, int* y_offset, int inactive_text_color) {
	if (item->required != NULL) {
		if (*(sbyte*)item->required == false) {
			return; // skip this item (disabled)
		}
	}

	rect_type text_rect = *parent;
	text_rect.top += *y_offset;
	int text_color = inactive_text_color;

	rect_type selection_box = text_rect;
	selection_box.bottom = selection_box.top + 8;
	selection_box.top -= 3;

	bool highlighted = (hovering_pause_menu_item == item->id);
	if (have_mouse_input && is_mouse_over_rect(&selection_box)) {
		hovering_pause_menu_item = item->id;
		highlighted = true;
	}

	if (highlighted) {
		previous_pause_menu_item = item->previous;
		next_pause_menu_item = item->next;
		// Skip over disabled items (such as the CHEATS menu in non-cheat mode)
		if (previous_pause_menu_item->required != NULL) {
			while (*(sbyte*)previous_pause_menu_item->required == false) {
				previous_pause_menu_item = previous_pause_menu_item->previous;
				if (previous_pause_menu_item->required == NULL) break;
			}
		}
		if (next_pause_menu_item->required != NULL) {
			while (*(sbyte*)next_pause_menu_item->required == false) {
				next_pause_menu_item = next_pause_menu_item->next;
				if (next_pause_menu_item->required == NULL) break;
			}
		}
		text_color = color_15_brightwhite;
		draw_rect_contours(&selection_box, color_7_lightgray);

		if (mouse_clicked) {
			if (is_mouse_over_rect(&selection_box)) {
				pause_menu_clicked(item);
			}
		} else if (pressed_enter && (drawn_menu == 0 || (drawn_menu == 1 && controlled_area == 0))) {
			pause_menu_clicked(item);
		}

	}
	show_text_with_color(&text_rect, 0, -1, item->text, text_color);
	*y_offset += 13;

}

void draw_pause_menu() {
	pause_menu_alpha = 120;
	draw_rect_with_alpha(&screen_rect, color_0_black, pause_menu_alpha);
	draw_rect_with_alpha(&rect_bottom_text, color_0_black, 0); // Transparent so that the text "GAME PAUSED" is visible.
	rect_type pause_rect_outer = {0, 110, 192, 210};
	rect_type pause_rect_inner;
	shrink2_rect(&pause_rect_inner, &pause_rect_outer, 5, 5);

	if (!have_mouse_input) {
		if (menu_control_y == 1) {
			play_menu_sound(sound_21_loose_shake_2);
			hovering_pause_menu_item = next_pause_menu_item->id;
		} else if (menu_control_y == -1) {
			play_menu_sound(sound_21_loose_shake_2);
			hovering_pause_menu_item = previous_pause_menu_item->id;
		}
	}

	int y_offset = 50;
	for (int i = 0; i < COUNT(pause_menu_items); ++i) {
		draw_pause_menu_item(&pause_menu_items[i], &pause_rect_inner, &y_offset, color_15_brightwhite);
	}
}

bool were_settings_changed;

void turn_setting_on_off(int setting_id, byte new_state, void* linked) {
	were_settings_changed = true;
	switch(setting_id) {
		default:
			if (linked != NULL) {
				*(byte*)(linked) = new_state;
			}
			break;
		case SETTING_FULLSCREEN:
			start_fullscreen = new_state;
			SDL_SetWindowFullscreen(window_, (new_state != 0) * SDL_WINDOW_FULLSCREEN_DESKTOP);
			break;
		case SETTING_USE_CORRECT_ASPECT_RATIO:
			use_correct_aspect_ratio = new_state;
			apply_aspect_ratio();
			break;
		case SETTING_USE_INTEGER_SCALING:
			use_integer_scaling = new_state;
			if (new_state) {
				window_resized();
			} else {
#if SDL_VERSION_ATLEAST(2,0,5) // SDL_RenderSetIntegerScale
				SDL_RenderSetIntegerScale(renderer_, SDL_FALSE);
#endif
			}
			break;
#ifdef USE_LIGHTING
		case SETTING_ENABLE_LIGHTING:
			enable_lighting = new_state;
			if (new_state && lighting_mask == NULL) {
				init_lighting();
			}
			need_full_redraw = 1;
			break;
#endif
		case SETTING_ENABLE_SOUND:
			turn_sound_on_off((new_state != 0) * 15);
			break;
		case SETTING_ENABLE_MUSIC:
			turn_music_on_off(new_state);
			break;
		case SETTING_USE_FIXES_AND_ENHANCEMENTS:
			turn_fixes_and_enhancements_on_off(new_state);
			break;
		case SETTING_USE_CUSTOM_OPTIONS:
			turn_custom_options_on_off(new_state);
			break;
	}
}

void turn_setting_on_off_with_sound(setting_type* setting, byte new_state) {
	play_menu_sound(sound_10_sword_vs_sword);
	turn_setting_on_off(setting->id, new_state, setting->linked);

}

int get_setting_value(setting_type* setting) {
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

void set_setting_value(setting_type* setting, int value) {
	if (setting->linked != NULL) {
		switch(setting->number_type) {
			default:
			case SETTING_BYTE:
				*(byte*) setting->linked = (byte) value;
				break;
			case SETTING_SBYTE:
				*(sbyte*) setting->linked = (sbyte) value;
				break;
			case SETTING_WORD:
				*(word*) setting->linked = (word) value;
				break;
			case SETTING_SHORT:
				*(short*) setting->linked = (short) value;
				break;
			case SETTING_INT:
				*(int*) setting->linked = value;
				break;
		}
	}
}

void increase_setting(setting_type* setting, int old_value) {
	int new_value;
	if (setting->id == SETTING_JOYSTICK_THRESHOLD) {
		new_value = ((old_value / 1000) + 1) * 1000; // Nearest higher multiple of 1000.
	} else {
		new_value = old_value + 1;
	}
	if (setting->linked != NULL && new_value <= setting->max) {
		were_settings_changed = true;
		set_setting_value(setting, new_value);
	}
}

void decrease_setting(setting_type* setting, int old_value) {
	int new_value;
	if (setting->id == SETTING_JOYSTICK_THRESHOLD) {
		new_value = (((old_value+999) / 1000) - 1) * 1000; // Nearest lower multiple of 1000.
	} else {
		new_value = old_value - 1;
	}
	if (setting->linked != NULL && new_value >= setting->min) {
		were_settings_changed = true;
		set_setting_value(setting, new_value);
	}
}


void draw_setting_explanation(setting_type* setting) {
	show_text_with_color(&explanation_rect, 0, -1, setting->explanation, color_7_lightgray);
}

void draw_image_with_blending(image_type far* image, int xpos, int ypos) {
	SDL_Rect src_rect = {0, 0, image->w, image->h};
	SDL_Rect dest_rect = {xpos, ypos, image->w, image->h};
	SDL_SetColorKey(image, SDL_TRUE, 0);
	if (SDL_BlitSurface(image, &src_rect, current_target_surface, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
}

#define print_setting_value(setting, value) print_setting_value_(setting, value, alloca(32), 32)
char* print_setting_value_(setting_type* setting, int value, char* buffer, size_t buffer_size) {
	bool has_name = false;
	names_list_type* list = setting->names_list;
	size_t max_len = MIN(MAX_OPTION_VALUE_NAME_LENGTH, buffer_size);
	if (list != NULL) {
		if (list->type == 0 && value >= 0 && value < list->names.count) {
			strncpy(buffer, (*(list->names.data))[value], max_len);
			has_name = true;
		} else if (list->type == 1) {
			for (int i = 0; i < list->kv_pairs.count; ++i) {
				key_value_type* kv_pair = list->kv_pairs.data + i;
				if (value == kv_pair->value) {
					strncpy(buffer, kv_pair->key, max_len);
					has_name = true;
					break;
				}
			}
		}
	}
	if (!has_name) {
		if (setting->id == SETTING_START_TICKS_LEFT ||
				setting->id == SETTING_SHIFT_L_REDUCED_TICKS ||
				setting->id == SETTING_MOUSE_DELAY ||
				setting->id == SETTING_LOOSE_FLOOR_DELAY
		) {
			float seconds = (float)value * (1.0f/12.0f);
			snprintf(buffer, buffer_size, "%.2f", seconds);
		} else {
			snprintf(buffer, buffer_size, "%d", value);
		}
	}
	return buffer;
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
		uint32_t rgb_color = SDL_MapRGBA(overlay_surface->format, 55, 55, 55, 255);
		if (SDL_FillRect(overlay_surface, &dest_rect, rgb_color) != 0) {
			sdlperror("draw_setting: SDL_FillRect");
			quit(1);
		}
		rect_type left_side_of_setting_box = setting_box;
		left_side_of_setting_box.left = setting_box.left - 2;
		left_side_of_setting_box.right = setting_box.left;
		draw_rect(&left_side_of_setting_box, color_15_brightwhite);
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
				if (!setting_enabled) {
					rect_type ON_hitbox = setting_box;
					ON_hitbox.left = setting_box.right - 22;
					if (is_mouse_over_rect(&ON_hitbox)) {
						turn_setting_on_off_with_sound(setting, 1);
						setting_enabled = false;
					}
				} else {
					rect_type OFF_hitbox = setting_box;
					OFF_hitbox.left = setting_box.right - 49;
					OFF_hitbox.right = setting_box.right - 22;
					if (is_mouse_over_rect(&OFF_hitbox)) {
						turn_setting_on_off_with_sound(setting, 0);
						setting_enabled = true;
					}
				}
			} else if (setting_enabled && menu_control_x < 0) {
				turn_setting_on_off_with_sound(setting, 0);
				setting_enabled = false;
			} else if (!setting_enabled && menu_control_x > 0) {
				turn_setting_on_off_with_sound(setting, 1);
				setting_enabled = true;
			}
		}

		int OFF_color = (setting_enabled) ? unselected_color : selected_color;
		int ON_color = (setting_enabled) ? selected_color : unselected_color;
		show_text_with_color(&text_rect, 1, -1, "ON", ON_color);
		text_rect.right -= 15;
		show_text_with_color(&text_rect, 1, -1, "OFF", OFF_color);

	} else if (setting->style == SETTING_STYLE_NUMBER && !disabled) {
		int value = get_setting_value(setting);
		if (highlighted_setting_id == setting->id) {
			if (mouse_clicked) {

				rect_type right_hitbox = {setting_box.top, text_rect.right - 5, setting_box.bottom, text_rect.right + 10};
				if (is_mouse_over_rect(&right_hitbox)) {
					increase_setting(setting, value);
				} else {
					char* value_text = print_setting_value(setting, value);
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

		value = get_setting_value(setting); // May have been updated.
		char* value_text = print_setting_value(setting, value);
		show_text_with_color(&text_rect, 1, -1, value_text, selected_color);

		if (highlighted_setting_id == setting->id) {
			int value_text_width = get_line_width(value_text, strlen(value_text));
			draw_image_with_blending(arrowhead_right_image, text_rect.right + 2, text_rect.top);
			draw_image_with_blending(arrowhead_left_image, text_rect.right - value_text_width - 6, text_rect.top);
		}

	} else {
		// show text only
		if (highlighted_setting_id == setting->id && (setting->required == NULL || *(sbyte*)setting->required != 0)) {
			if (pressed_enter || (mouse_clicked && is_mouse_over_rect(&setting_box))) {
				if (setting->id == SETTING_RESET_ALL_SETTINGS) {
					play_menu_sound(sound_22_loose_shake_3);
					current_dialog_box = DIALOG_RESTORE_DEFAULT_SETTINGS;
					current_dialog_text = "Restore all settings to their default values?";
				} else if (setting->id == SETTING_LEVEL_SETTINGS) {
					play_menu_sound(sound_22_loose_shake_3);
					current_dialog_box = DIALOG_SELECT_LEVEL;
				}
			}

		}
	}

	*y_offset += 15;
}

void menu_scroll(int y) {
	settings_area_type* current_settings_area = get_settings_area(active_settings_subsection);
	if (current_settings_area != NULL) {
		int max_scroll = MAX(0, current_settings_area->setting_count - 9);
		if (drawn_menu == 1 && controlled_area == 1) {
			if (y < 0 && scroll_position > 0) {
				--scroll_position;
			} else if (y > 0 && scroll_position < max_scroll) {
				++scroll_position;
			}
		}
	}
}

void draw_settings_area(settings_area_type* settings_area) {
	if (settings_area == NULL) return;
	rect_type settings_area_rect = {0, 80, 170, 320};
	shrink2_rect(&settings_area_rect, &settings_area_rect, 20, 20);
	int y_offset = 0;
	int num_drawn_settings = 0;

	// The MODS subsection with level specific settings is a special case:
	// We want to display which level we are editing, and start drawing slightly lower.
	if (active_settings_subsection == SETTINGS_MENU_LEVEL_CUSTOMIZATION) {
		y_offset = 15;
		char level_text[16];
		snprintf(level_text, sizeof(level_text), "LEVEL %d", menu_current_level);
		show_text_with_color(&settings_area_rect, 0, -1, level_text, color_15_brightwhite);
	}

	for (int i = 0; (i < settings_area->setting_count) && (num_drawn_settings < 9); ++i) {
		if (i >= scroll_position) {
			++num_drawn_settings;
			draw_setting(&settings_area->settings[i], &settings_area_rect, &y_offset, color_15_brightwhite);
		}
	}
	if (scroll_position > 0) {
		draw_image_with_blending(arrowhead_up_image, 200, 10);
	}
	if (scroll_position + num_drawn_settings < settings_area->setting_count) {
		draw_image_with_blending(arrowhead_down_image, 200, 151);
	}

	// Draw a scroll bar if needed.
	// It's not clickable yet, it just shows where you are in the list.
	if (num_drawn_settings < settings_area->setting_count) {
		const int scrollbar_width = 2;
		rect_type scrollbar_rect = {
			.top = settings_area_rect.top - 5, .bottom = settings_area_rect.bottom,
			.left = settings_area_rect.right + 10 - scrollbar_width, .right = settings_area_rect.right + 10
		};
		method_5_rect(&scrollbar_rect, blitters_0_no_transp, color_8_darkgray);

		int scrollbar_height = scrollbar_rect.bottom - scrollbar_rect.top;
		rect_type scrollbar_slider_rect = {
			.top = scrollbar_rect.top + scroll_position * scrollbar_height / settings_area->setting_count,
			.bottom = scrollbar_rect.top + (scroll_position + num_drawn_settings) * scrollbar_height / settings_area->setting_count,
			.left = scrollbar_rect.left, .right = scrollbar_rect.right
		};
		method_5_rect(&scrollbar_slider_rect, blitters_0_no_transp, color_7_lightgray);
	}
}

void draw_settings_menu() {
	settings_area_type* settings_area = get_settings_area(active_settings_subsection);
	pause_menu_alpha = (settings_area == NULL) ? 220 : 255;
	draw_rect_with_alpha(&screen_rect, color_0_black, pause_menu_alpha);

	rect_type pause_rect_outer = {0, 10, 192, 80};
	rect_type pause_rect_inner;
	shrink2_rect(&pause_rect_inner, &pause_rect_outer, 5, 5);

	if (!have_mouse_input) {
		bool hovering_item_changed = false;
		if (controlled_area == 0) {
			int old_hovering_item_id = hovering_pause_menu_item;
			if (menu_control_y == 1) {
				hovering_pause_menu_item = next_pause_menu_item->id;
			} else if (menu_control_y == -1) {
				hovering_pause_menu_item = previous_pause_menu_item->id;
			}
			if (old_hovering_item_id != hovering_pause_menu_item) {
				hovering_item_changed = true;
			}
		} else if (controlled_area == 1) {
			// settings area
			int old_highlighted_setting_id = highlighted_setting_id;

			// Why does the global variable contain the ID instead of the index?...
			// Find the index from the ID.
			settings_area_type* current_settings_area = get_settings_area(active_settings_subsection);
			int highlighted_setting_index = -1;
			for (int i = 0; i < current_settings_area->setting_count; i++) {
				if (highlighted_setting_id == current_settings_area->settings[i].id) {
					highlighted_setting_index = i;
					break;
				}
			}

			int last = current_settings_area->setting_count - 1;
			int max_scroll = MAX(0, current_settings_area->setting_count - 9);

			if (menu_control_y > 0) {
				// DOWN
				highlighted_setting_index += menu_control_y;
				if (highlighted_setting_index > last) highlighted_setting_index = last;

				// With Page Down, try to leave the selection in the same row visually.
				if (menu_control_y > +1) scroll_position += menu_control_y;

			} else if (menu_control_y < 0) {
				// UP
				highlighted_setting_index += menu_control_y;
				if (highlighted_setting_index < 0) highlighted_setting_index = 0;

				// With Page Up, try to leave the selection in the same row visually.
				if (menu_control_y < -1) scroll_position += menu_control_y;

			}

			if (menu_control_y != 0) {
				// We check both directions in both cases, to scroll the highlighted row back into sight even if the user scrolled it out of sight (with the mouse wheel).
				if (highlighted_setting_index - 8 > scroll_position) scroll_position = highlighted_setting_index - 8;
				if (highlighted_setting_index < scroll_position) scroll_position = highlighted_setting_index;
				if (scroll_position > max_scroll) scroll_position = max_scroll;
				if (scroll_position < 0) scroll_position = 0;
			}

			// Find the ID from the index.
			highlighted_setting_id = current_settings_area->settings[highlighted_setting_index].id;

			if (old_highlighted_setting_id != highlighted_setting_id) {
				hovering_item_changed = true;
			}
		}
		if (hovering_item_changed) {
			play_menu_sound(sound_21_loose_shake_2);
		}
	}

	int y_offset = 50;
	for (int i = 0; i < COUNT(settings_menu_items); ++i) {
		pause_menu_item_type* item = &settings_menu_items[i];
		int text_color = (highlighted_settings_subsection == item->id) ? color_15_brightwhite : color_7_lightgray;
		draw_pause_menu_item(&settings_menu_items[i], &pause_rect_inner, &y_offset, text_color);
	}

	draw_settings_area(settings_area);
}

enum dialog_button_ids {
	DIALOG_BUTTON_CANCEL,
	DIALOG_BUTTON_OK,
};

void confirmation_dialog_result(int which_dialog, int button) {
	if (button == DIALOG_BUTTON_OK) {
		if (which_dialog == DIALOG_RESTORE_DEFAULT_SETTINGS) {
			play_menu_sound(sound_10_sword_vs_sword);
			were_settings_changed = true;
			set_options_to_default();
			turn_setting_on_off(SETTING_USE_INTEGER_SCALING, use_integer_scaling, NULL);
#ifdef USE_LIGHTING
			turn_setting_on_off(SETTING_ENABLE_LIGHTING, enable_lighting, NULL);
#endif
			apply_aspect_ratio();
			turn_sound_on_off((is_sound_on != 0) * 15);
			turn_music_on_off(enable_music);
		} else if (which_dialog == DIALOG_CONFIRM_QUIT) {
			last_key_scancode = SDL_SCANCODE_Q | WITH_CTRL;
			key_test_quit();
		}
	} else {
		play_menu_sound(sound_22_loose_shake_3);
	}
}

rect_type cancel_text_rect = {104, 162,  118,  212};
rect_type cancel_highlight_rect = {103, 162,  116,  212};
rect_type ok_text_rect = {104, 108,  118,  158};
rect_type ok_highlight_rect = {103, 108,  116,  158};

void draw_confirmation_dialog(int which_dialog, const char* text) {
	int highlighted_button = DIALOG_BUTTON_OK;
	int old_highlighted_button = -1;
	for (;;) {
		process_events();
		key_test_paused_menu(key_test_quit());
		process_additional_menu_input();

		if (menu_control_back == 1) {
			confirmation_dialog_result(which_dialog, DIALOG_BUTTON_CANCEL);
			break;
		}

		if (have_mouse_input) {
			if (is_mouse_over_rect(&ok_highlight_rect)) {
				highlighted_button = DIALOG_BUTTON_OK;
			} else if (is_mouse_over_rect(&cancel_highlight_rect)) {
				highlighted_button = DIALOG_BUTTON_CANCEL;
			}
		}

		if (menu_control_x < 0) {
			highlighted_button = DIALOG_BUTTON_OK;
		} else if (menu_control_x > 0) {
			highlighted_button = DIALOG_BUTTON_CANCEL;
		} else if (mouse_clicked || pressed_enter) {
			confirmation_dialog_result(which_dialog, highlighted_button);
			break;
		}

		if (highlighted_button != old_highlighted_button) {
			old_highlighted_button = highlighted_button;
			// Need to redraw the dialog box.
			uint32_t clear_color = SDL_MapRGBA(current_target_surface->format, 0, 0, 0, 255);
			SDL_FillRect(overlay_surface, NULL, clear_color);
			draw_rect(&copyprot_dialog->peel_rect, color_0_black);
			dialog_method_2_frame(copyprot_dialog);
			rect_type rect;
			shrink2_rect(&rect, &copyprot_dialog->text_rect, 2, 1);
			rect.bottom -= 14;
			show_text_with_color(&rect, 0, 0, text, color_15_brightwhite);
			clear_kbd_buf();

			rect_type* highlight_rect;
			int ok_text_color, cancel_text_color;
			if (highlighted_button == DIALOG_BUTTON_OK) {
				highlight_rect = &ok_highlight_rect;
				ok_text_color = color_15_brightwhite;
				cancel_text_color = color_7_lightgray;
			} else {
				highlight_rect = &cancel_highlight_rect;
				ok_text_color = color_7_lightgray;
				cancel_text_color = color_15_brightwhite;
			}
			draw_rect(highlight_rect, color_8_darkgray);
			show_text_with_color(&ok_text_rect, 0, 0, "OK", ok_text_color);
			show_text_with_color(&cancel_text_rect, 0, 0, "Cancel", cancel_text_color);
			update_screen();
		}

		SDL_Delay(1); // Prevent 100% cpu usage.

	}
	current_dialog_box = 0;
	clear_menu_controls();
}

void draw_select_level_dialog() {
	clear_menu_controls();
	int old_edited_level_number = -1;
	for (;;) {
		process_events();
		key_test_paused_menu(key_test_quit());
		process_additional_menu_input();

		if (menu_control_back == 1) {
			menu_control_back = 0;
			play_menu_sound(sound_22_loose_shake_3);
			break;
		}

		if (menu_control_x < 0) {
			menu_current_level = MAX(0, menu_current_level - 1);
		} else if (menu_control_x > 0) {
			menu_current_level = MIN(15, menu_current_level + 1);
		} else if (mouse_clicked || pressed_enter) {
			enter_settings_subsection(SETTINGS_MENU_LEVEL_CUSTOMIZATION);
			highlighted_settings_subsection = SETTINGS_MENU_MODS;
			play_menu_sound(sound_22_loose_shake_3);
			break;
		}

		if (menu_current_level != old_edited_level_number) {
			font_type* saved_font = textstate.ptr_font;
			textstate.ptr_font = &hc_font;

			old_edited_level_number = menu_current_level;
			// Need to redraw the dialog box.
			uint32_t clear_color = SDL_MapRGBA(current_target_surface->format, 0, 0, 0, 255);
			SDL_FillRect(overlay_surface, NULL, clear_color);
			draw_rect(&copyprot_dialog->peel_rect, color_0_black);
			dialog_method_2_frame(copyprot_dialog);
			rect_type rect;
			shrink2_rect(&rect, &copyprot_dialog->text_rect, 2, 1);
			rect.bottom -= 14;
			show_text_with_color(&rect, 0, 0, "Customize level...", color_15_brightwhite);
			clear_kbd_buf();
			rect_type input_rect = {104,   64,  118,  256};
			char level_text[8];
			snprintf(level_text, sizeof(level_text), "%d", menu_current_level);
			show_text_with_color(&input_rect, 0, 0, level_text, color_15_brightwhite);
			draw_image_with_blending(arrowhead_right_image, 175, input_rect.top + 3);
			draw_image_with_blending(arrowhead_left_image, 145 - 3, input_rect.top + 3);

			update_screen();
			textstate.ptr_font = saved_font;
		}

		SDL_Delay(1); // Prevent 100% cpu usage.

	}
	clear_menu_controls();
}

int need_full_menu_redraw_count;

void draw_menu() {
	escape_key_suppressed = (key_states[SDL_SCANCODE_BACKSPACE] || key_states[SDL_SCANCODE_ESCAPE]);
	surface_type* saved_target_surface = current_target_surface;
	current_target_surface = overlay_surface;

	need_close_menu = false;
	while (!need_close_menu) {
		clear_menu_controls();
		process_events();
		if (process_key() != 0) {
			break; // Menu was forcefully closed, for example by pressing Ctrl+A.
		}
		process_additional_menu_input();

		if (current_dialog_box != DIALOG_NONE) {
			if (current_dialog_box == DIALOG_SELECT_LEVEL) {
				draw_select_level_dialog();
			} else {
				draw_confirmation_dialog(current_dialog_box, current_dialog_text);
			}
			current_dialog_box = DIALOG_NONE;
			clear_menu_controls();
		}

		if (is_menu_shown == 1) {
			is_menu_shown = -1; // reset the menu if the menu is drawn for the first time
			need_full_menu_redraw_count = 2;
			reset_paused_menu();
		}
		if (menu_control_back == 1) {
			play_menu_sound(sound_22_loose_shake_3);
			if (drawn_menu == 1) {
				if (controlled_area == 1) {
					leave_settings_subsection();
				} else {
					reset_paused_menu(); // Go back to the top level pause menu.
				}
			} else {
				break; // Close the menu.
			}
		}

		if (menu_control_scroll_y != 0) {
			menu_scroll(menu_control_scroll_y);
		}

		if (have_mouse_input || have_keyboard_or_controller_input) {
			// The menu is updated+drawn within the same routine, so redrawing may be necessary after the first time.
			// TODO: Maybe in the future fully separate updating from drawing?
			need_full_menu_redraw_count = 2;
		} else {
			if (need_full_menu_redraw_count == 0) {
				SDL_Delay(1);
				continue; // Don't redraw if there is no input to process (save CPU cycles).
			}
		}

//		Uint64 begin = SDL_GetPerformanceCounter();
		font_type* saved_font = textstate.ptr_font;
		textstate.ptr_font = &hc_small_font;
		if (drawn_menu == 0) {
			draw_pause_menu();
		} else if (drawn_menu == 1) {
			draw_settings_menu();
		}
		textstate.ptr_font = saved_font;
		if (!need_close_menu) {
			update_screen();
		}
//		printf("Drawing the menu took %.2f ms.\n", (SDL_GetPerformanceCounter() - begin) * milliseconds_per_counter);

		--need_full_menu_redraw_count;
	}

	current_target_surface = saved_target_surface;
}

void clear_menu_controls() {
	pressed_enter = 0;
	mouse_moved = 0;
	mouse_clicked = 0;
	mouse_button_clicked_right = 0;
	have_mouse_input = 0;
	have_keyboard_or_controller_input = 0;
	menu_control_x = 0;
	menu_control_y = 0;
	menu_control_back = 0;
	menu_control_scroll_y = 0;
}

void process_additional_menu_input() {
	read_mouse_state();
	have_keyboard_or_controller_input = (menu_control_x || menu_control_y || menu_control_back || pressed_enter);
	have_mouse_input = (mouse_moved || mouse_clicked || mouse_button_clicked_right || menu_control_scroll_y);

	dword flags = SDL_GetWindowFlags(window_);
	if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		if (have_mouse_input) {
			SDL_ShowCursor(SDL_ENABLE);
		} else if (have_keyboard_or_controller_input) {
			SDL_ShowCursor(SDL_DISABLE);
		}
	} else {
		SDL_ShowCursor(SDL_ENABLE);
	}
}

bool joy_ABXY_buttons_released;
bool joy_xy_released;
Uint64 joy_xy_timeout_counter;

int key_test_paused_menu(int key) {
	menu_control_x = 0;
	menu_control_y = 0;
	menu_control_back = 0;

	if (mouse_button_clicked_right) {
		menu_control_back = 1; // Can use RMB to close menus.
	}

	if (is_joyst_mode) {
		int joy_x = joy_hat_states[0];
		int joy_y = joy_hat_states[1];
		int y_threshold = 14000;
		int x_threshold = 26000; // Less sensitive, to prevent accidentally changing a setting.
		if (joy_axis[SDL_CONTROLLER_AXIS_LEFTY] < -y_threshold) {
			joy_y = -1;
		} else if (joy_axis[SDL_CONTROLLER_AXIS_LEFTY] > y_threshold) {
			joy_y = 1;
		} else if (joy_axis[SDL_CONTROLLER_AXIS_LEFTX] < -x_threshold) {
			joy_x = -1;
		} else if (joy_axis[SDL_CONTROLLER_AXIS_LEFTX] > x_threshold) {
			joy_x = 1;
		}

		float needed_timeout_s = 0.1f; // Delay for hold-down repeated input.
		if (joy_x == 0 && joy_y == 0) {
			joy_xy_released = true;
			joy_xy_timeout_counter = 0;
		} else {
			if (joy_xy_released) {
				needed_timeout_s = 0.3f; // The delay is longer for the first repetition.
				joy_xy_released = false;
			}
			Uint64 current_counter = SDL_GetPerformanceCounter();
			if (current_counter > joy_xy_timeout_counter) {
				menu_control_x = joy_x;
				menu_control_y = joy_y;
				joy_xy_timeout_counter = current_counter + (Uint64)((float)SDL_GetPerformanceFrequency() * needed_timeout_s);
				return 0; // cancel other input.
			}
		}

		if (joy_AY_buttons_state == 0 && joy_B_button_state == 0) {
			joy_ABXY_buttons_released = true;
		} else if (joy_ABXY_buttons_released) {
			joy_ABXY_buttons_released = false;
			if (joy_AY_buttons_state == 1 /* A pressed */) {
				key = SDL_SCANCODE_RETURN;
				joy_AY_buttons_state = 0; // Prevent 'down' input being passed to the controls if the game is unpaused.
			} else if (joy_B_button_state == 1) {
				key = SDL_SCANCODE_ESCAPE;
			}
		}
	}

	switch(key) {
		default:
			if (key & WITH_CTRL) {
				need_close_menu = true;
				return key; // Allow Ctrl+R, etc.
			} else {
				break;
			}
		case SDL_SCANCODE_UP:
			menu_control_y = -1;
			break;
		case SDL_SCANCODE_DOWN:
			menu_control_y = 1;
			break;
		case SDL_SCANCODE_PAGEUP:
			menu_control_y = -9;
			break;
		case SDL_SCANCODE_PAGEDOWN:
			menu_control_y = +9;
			break;
		case SDL_SCANCODE_HOME:
			menu_control_y = -1000;
			break;
		case SDL_SCANCODE_END:
			menu_control_y = +1000;
			break;
		case SDL_SCANCODE_RIGHT:
			menu_control_x = 1;
			break;
		case SDL_SCANCODE_LEFT:
			menu_control_x = -1;
			break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
			pressed_enter = 1;
			break;
		case SDL_SCANCODE_ESCAPE:
		case SDL_SCANCODE_BACKSPACE:
			menu_control_back = 1;
			break;
		case SDL_SCANCODE_F6:
		case SDL_SCANCODE_F6 | WITH_SHIFT:
			if (Kid.alive < 0) need_quick_save = 1;
			need_close_menu = true;
			break;
		case SDL_SCANCODE_F9:
		case SDL_SCANCODE_F9 | WITH_SHIFT:
			need_quick_load = 1;
			need_close_menu = true;
			break;
	}
	return 0;
}

typedef int rw_process_func_type(SDL_RWops* rw, void* data, size_t data_size);

// For serializing/unserializing options in the in-game settings menu
#define process(x) if (!process_func(rw, &(x), sizeof(x))) return
void process_ingame_settings_user_managed(SDL_RWops* rw, rw_process_func_type process_func) {
	process(enable_pause_menu);
	process(enable_info_screen);
	process(is_sound_on);
	process(enable_music);
	process(enable_controller_rumble);
	process(joystick_threshold);
	process(joystick_only_horizontal);
	process(enable_replay);
	process(start_fullscreen);
	process(use_correct_aspect_ratio);
	process(use_integer_scaling);
	process(scaling_type);
	process(enable_fade);
	process(enable_flash);
#ifdef USE_LIGHTING
	process(enable_lighting);
#endif
}

void process_ingame_settings_mod_managed(SDL_RWops* rw, rw_process_func_type process_func) {
	process(enable_copyprot);
	process(enable_quicksave);
	process(enable_quicksave_penalty);
	process(use_fixes_and_enhancements);
	process(fixes_saved);
	process(use_custom_options);
	process(custom_saved);
}
#undef process

// CRC-32 implementation adapted from:
// http://www.hackersdelight.org/hdcodetxt/crc.c.txt
unsigned int crc32c(unsigned char *message, size_t size) {
	int i, j;
	unsigned int byte, crc, mask;
	static unsigned int table[256];

	/* Set up the table, if necessary. */
	if (table[1] == 0) {
		for (byte = 0; byte <= 255; byte++) {
			crc = byte;
			for (j = 7; j >= 0; j--) {    // Do eight times.
				mask = -(crc & 1);
				crc = (crc >> 1) ^ (0xEDB88320 & mask);
			}
			table[byte] = crc;
		}
	}

	/* Through with table setup, now calculate the CRC. */
	i = 0;
	crc = 0xFFFFFFFF;
	while ((byte = message[i]), size--) {
		crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
		i = i + 1;
	}
	return ~crc;
}

dword exe_crc = 0;

void calculate_exe_crc() {
	if (exe_crc == 0) {
		// Get the CRC32 fingerprint of the executable.
		FILE* exe_file = fopen(g_argv[0], "rb");
		if (exe_file != NULL) {
			fseek(exe_file, 0, SEEK_END);
			size_t size = ftell(exe_file);
			fseek(exe_file, 0, SEEK_SET);
			if (size > 0) {
				byte* buffer = malloc(size);
				size_t bytes = fread(buffer, 1, size, exe_file);
				if (bytes != size) {
					fprintf(stderr, "exec changed size during CRC32!?\n");
					size = bytes;
				}
				exe_crc = crc32c(buffer, size);
				free(buffer);
			}
			fclose(exe_file);
		}
	}
}

void save_ingame_settings() {
	SDL_RWops* rw = SDL_RWFromFile(locate_file("SDLPoP.cfg"), "wb");
	if (rw != NULL) {
		calculate_exe_crc();
		SDL_RWwrite(rw, &exe_crc, sizeof(exe_crc), 1);
		byte levelset_name_length = (byte)strnlen(levelset_name, UINT8_MAX);
		SDL_RWwrite(rw, &levelset_name_length, sizeof(levelset_name_length), 1);
		SDL_RWwrite(rw, levelset_name, levelset_name_length, 1);
		process_ingame_settings_user_managed(rw, process_rw_write);
		process_ingame_settings_mod_managed(rw, process_rw_write);
		SDL_RWclose(rw);
	}
}

void load_ingame_settings() {
	// We want the SDLPoP.cfg file (in-game menu settings) to override the SDLPoP.ini file,
	// but ONLY if the .ini file wasn't modified since the last time the .cfg file was saved!
	struct stat st_ini, st_cfg;
	const char* cfg_filename = locate_file("SDLPoP.cfg");
	const char* ini_filename = locate_file("SDLPoP.ini");
	if (stat( cfg_filename, &st_cfg ) == 0 && stat( ini_filename, &st_ini ) == 0) {
		if (st_ini.st_mtime > st_cfg.st_mtime ) {
			// SDLPoP.ini is newer than SDLPoP.cfg, so just go with the .ini configuration
			return;
		}
	}
	// If there is a SDLPoP.cfg file, let it override the settings
	SDL_RWops* rw = SDL_RWFromFile(cfg_filename, "rb");
	if (rw != NULL) {
		// SDLPoP.cfg should be invalidated if the prince executable changes.
		// This allows us not to worry about future and backward compatibility of this file.
		calculate_exe_crc();
		dword expected_crc = 0;
		SDL_RWread(rw, &expected_crc, sizeof(expected_crc), 1);
//		printf("CRC-32: exe = %x, expected = %x\n", exe_crc, expected_crc);
		if (exe_crc == expected_crc) {
			byte cfg_levelset_name_length;
			char cfg_levelset_name[256] = {0};
			SDL_RWread(rw, &cfg_levelset_name_length, sizeof(cfg_levelset_name_length), 1);
			SDL_RWread(rw, cfg_levelset_name, cfg_levelset_name_length, 1);
//			printf("%s, %s\n", cfg_levelset_name, levelset_name);
			process_ingame_settings_user_managed(rw, process_rw_read); // Load the settings.
			// For mod-managed settings: discard the CFG settings when switching to different mod.
			if (strncmp(levelset_name, cfg_levelset_name, 256) == 0) {
				process_ingame_settings_mod_managed(rw, process_rw_read);
			}
		}
		SDL_RWclose(rw);
	}
}

void menu_was_closed() {
	is_paused = 0;
	is_menu_shown = 0;
	escape_key_suppressed = (key_states[SDL_SCANCODE_BACKSPACE] || key_states[SDL_SCANCODE_ESCAPE]);
	if (were_settings_changed) {
		save_ingame_settings();
		were_settings_changed = false;
	}
	// In fullscreen mode, hide the mouse cursor (because it is only needed in the menu).
	dword flags = SDL_GetWindowFlags(window_);
	if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		SDL_ShowCursor(SDL_DISABLE);
	} else {
		SDL_ShowCursor(SDL_ENABLE);
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
