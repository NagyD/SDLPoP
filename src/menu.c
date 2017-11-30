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
int active_settings_menu_item = 0;
int menu_control_y;
int menu_control_x;

enum menu_setting_style_ids {
	SETTING_STYLE_TOGGLE,
	SETTING_STYLE_SLIDER,
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
};

typedef struct setting_type {
	int id;
	int previous, next;
	int style;
	void* linked;
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
				.text = "Enable replays", .explanation = "Enable recording/replay feature.\n"
						"Press Ctrl+Tab in-game to start recording.\n"
						"To stop, press Ctrl+Tab again."},
		{.id = SETTING_USE_FIXES_AND_ENHANCEMENTS, .style = SETTING_STYLE_TOGGLE, .linked = &use_fixes_and_enhancements,
				.text = "Enhanced mode (allow bug fixes)"},
		{.id = SETTING_ENABLE_CROUCH_AFTER_CLIMBING, .style = SETTING_STYLE_TOGGLE, .linked = &enable_crouch_after_climbing,
				.text = "Enable crouching after climbing"},
		{.id = SETTING_ENABLE_FREEZE_TIME_DURING_END_MUSIC, .style = SETTING_STYLE_TOGGLE, .linked = &enable_freeze_time_during_end_music,
				.text = "Freeze time during level end music"},
		{.id = SETTING_ENABLE_REMEMBER_GUARD_HP, .style = SETTING_STYLE_TOGGLE, .linked = &enable_remember_guard_hp,
				.text = "Remember guard hitpoints"},
		{.id = SETTING_FIX_GATE_SOUNDS, .style = SETTING_STYLE_TOGGLE, .linked = &fix_gate_sounds,
				.text = "Fix gate sounds bug"},
		{.id = SETTING_TWO_COLL_BUG, .style = SETTING_STYLE_TOGGLE, .linked = &fix_two_coll_bug,
				.text = "Fix two collisions bug"},
		{.id = SETTING_FIX_INFINITE_DOWN_BUG, .style = SETTING_STYLE_TOGGLE, .linked = &fix_infinite_down_bug,
				.text = "Fix infinite down bug"},
};

setting_type mods_settings[] = {
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Starting minutes left"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Starting hitpoints"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Max hitpoints allowed"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Saving allowed: first level"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Saving allowed: last level"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Start with the screen flipped"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Start in blind mode"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Copy protection before level"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Allow triggering any tile"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "Enable WDA in palace"},
		{.id = 1, .style = SETTING_STYLE_SLIDER, .text = "First level"},
};


int menu_padding = 3;
int menu_spacer_height = 3;
int menu_item_height = 8;

void add_menu_item(menu_type* menu, menu_item_type new_item) {
	if (menu->num_items < MAX_MENU_ITEMS) {
		menu_item_type* item = &menu->items[menu->num_items++];
		*item = new_item;

		if (new_item.is_spacer) {
			menu->height += menu_spacer_height;
		} else {
			menu->height += menu_item_height;
		}

		int item_text_1_width = get_line_width(new_item.text_left, strlen(new_item.text_left));
		int item_text_2_width = get_line_width(new_item.text_right, strlen(new_item.text_right));
		int item_text_total_width = item_text_1_width + item_text_2_width + 10;

		if (item_text_total_width > menu->max_item_text_width) {
			menu->max_item_text_width = item_text_total_width;
		}
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
			item->previous = (first_setting + MAX(0, i-1))->id;
			item->next = (first_setting + MIN(setting_count-1, i+1))->id;
		}
		setting_type* last_item = first_setting + (setting_count-1);
		first_setting->previous = last_item->id;
		last_item->next = first_setting->id;
	}
}


void init_menu() {
	init_pause_menu_items(pause_menu_items, COUNT(pause_menu_items));
	init_pause_menu_items(settings_menu_items, COUNT(settings_menu_items));

	init_settings_list(general_settings, COUNT(general_settings));
	init_settings_list(visuals_settings, COUNT(visuals_settings));
	init_settings_list(gameplay_settings, COUNT(gameplay_settings));
	init_settings_list(mods_settings, COUNT(mods_settings));

	menubar_state = 0;

	font_type* saved_font = textstate.ptr_font;
	textstate.ptr_font = &small_font;

	add_menu_item(&game_menu, (menu_item_type){.text_left = "Restart game", .text_right = "Ctrl+R"});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Restart level", .text_right = "Ctrl+A"});
	add_menu_item(&game_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Levelset", .text_right = ">"});
	add_menu_item(&game_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Save game...", .text_right = ""});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Load game...", .text_right = ""});
	add_menu_item(&game_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Quicksave", .text_right = "F6"});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Quickload", .text_right = "F9"});
	add_menu_item(&game_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Load replay...", .text_right = ""});
	add_menu_item(&game_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&game_menu, (menu_item_type){.text_left = "Quit", .text_right = "Ctrl+Q"});

	add_menu_item(&options_menu, (menu_item_type){.text_left = "Video", .text_right = ">"});
	add_menu_item(&options_menu, (menu_item_type){.text_left = "Sound", .text_right = ">"});
	add_menu_item(&options_menu, (menu_item_type){.text_left = "Controller", .text_right = ">"});
	add_menu_item(&options_menu, (menu_item_type){.text_left = "Bug fixes / enhancements", .text_right = ">"});
	add_menu_item(&options_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&options_menu, (menu_item_type){.text_left = "Edit configuration file...", .text_right = ""});
	add_menu_item(&options_menu, (menu_item_type){.text_left = "Reload configuration file", .text_right = ""});

	add_menu_item(&capture_menu, (menu_item_type){.text_left = "Screenshot", .text_right = "Shift+F12"});
	add_menu_item(&capture_menu, (menu_item_type){.text_left = "Level screenshot", .text_right = "Ctrl+Shift+F12"});
	add_menu_item(&capture_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&capture_menu, (menu_item_type){.text_left = "Replay", .text_right = "Ctrl+Tab"});
	add_menu_item(&capture_menu, (menu_item_type){.text_left = "Record from", .text_right = ">"});

	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Enable cheats", .text_right = ""});
	add_menu_item(&cheat_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Next level", .text_right = "Shift+L"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Health", .text_right = "Shift+S"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Life", .text_right = "Shift+T"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Feather fall", .text_right = "Shift+W"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Resurrect", .text_right = "R"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Kill guard", .text_right = "K"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Increase time", .text_right = "+"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Decrease time", .text_right = "-"});
	add_menu_item(&cheat_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Look at room", .text_right = ">"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Show adjacent rooms", .text_right = "C"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Show diagonal rooms", .text_right = "Shift+C"});
	add_menu_item(&cheat_menu, (menu_item_type){.is_spacer = 1});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Flip screen", .text_right = "Shift+I"});
	add_menu_item(&cheat_menu, (menu_item_type){.text_left = "Blind mode", .text_right = "Shift+B"});

	textstate.ptr_font = saved_font;
}

bool is_mouse_over_rect(rect_type* rect) {
	return (mouse_x >= rect->left && mouse_x < rect->right && mouse_y >= rect->top && mouse_y < rect->bottom);
}

void draw_menu_item(menu_type* menu, rect_type* menu_rect, menu_item_type* item, int item_index, int* offset_y) {
	rect_type item_box_rect = *menu_rect;
	int height = (item->is_spacer) ? menu_spacer_height : menu_item_height;

	item_box_rect.top += *offset_y;
	item_box_rect.bottom = item_box_rect.top + height;

	bool hovering = (menu->hovering_item_index == item_index);
	if (!hovering && !item->is_spacer) {
		hovering = is_mouse_over_rect(&item_box_rect);
	}

	if (hovering) {
		menu->hovering_item_index = item_index;
		draw_rect_with_alpha(&item_box_rect, color_7_lightgray, menu_alpha);
	}

	if (item->is_spacer) {
		rect_type spacer_rect = item_box_rect;
		spacer_rect.left += menu_padding;
		spacer_rect.right -= menu_padding;
		spacer_rect.top += 1;
		spacer_rect.bottom -= 1;
		draw_rect_with_alpha(&spacer_rect, color_8_darkgray, menu_alpha);
	} else {
		rect_type item_text_rect = item_box_rect;
		item_text_rect.left += menu_padding;
		item_text_rect.right -= menu_padding;
		item_text_rect.top += 1;
		item_text_rect.bottom += 1;
		int text_color = (hovering) ? color_0_black : color_15_brightwhite;
		show_text_with_color(&item_text_rect, -1, -1, item->text_left, text_color);
		show_text_with_color(&item_text_rect, 1, -1, item->text_right, text_color);
	}

	*offset_y += height;
}

void draw_menu(menu_type* menu, int left) {
	rect_type menu_rect;
	menu_rect.top = menubar_rect.bottom;
	menu_rect.left = left;
	menu_rect.bottom = menu_rect.top + menu->height;
	menu_rect.right = left + menu->max_item_text_width + 2*menu_padding;
	draw_rect_with_alpha(&menu_rect, color_0_black, menu_alpha);

	int offset_y = 0;
	for (int i = 0; i < menu->num_items; ++i) {
		menu_item_type* item = &menu->items[i];
		draw_menu_item(menu, &menu_rect, item, i, &offset_y);
	}

}

void draw_menubar_item(menubar_item_type* menubar_item, int* offset_x) {

	int padding = 3;
	int text_width = get_line_width(menubar_item->text, strlen(menubar_item->text));
	int box_width = text_width + 2*padding;
	int box_left = *offset_x;
	rect_type box_rect = {0, box_left, menubar_rect.bottom, box_left + box_width};

	int item_state = 0;
	if (menubar_state == 1) {
		if (menubar_item->id == selected_menu_id && mouse_y >= menubar_rect.bottom) {
			item_state = 1; // menu is already selected
		} else if (is_mouse_over_rect(&box_rect)) {
			// Select this menu instead of another one
			selected_menu_id = menubar_item->id;
			menubar_item->associated_menu->hovering_item_index = -1;
			item_state = 1;
		}
	} else if (menubar_state == 0 && is_mouse_over_rect(&box_rect)) {
		item_state = 2;
	}

	bool draw_box;
	byte box_color;
	int text_color;
	if (item_state == 1) { // menu is selected
		draw_box = true;
		box_color = color_0_black;
		text_color = color_15_brightwhite;
	} else if (item_state == 2) { // menu is being hovered over
		draw_box = true;
		box_color = color_7_lightgray;
		text_color = color_0_black;
	} else {
		draw_box = false;
		box_color = color_7_lightgray;
		text_color = color_0_black;
	}

	if (draw_box) {
		draw_rect_with_alpha(&box_rect, box_color, menu_alpha);
	}
	rect_type text_rect = box_rect;
	text_rect.left += padding;
	text_rect.top += 1; // prevent text from touching the very top of the screen
	text_rect.bottom += 1;
	show_text_with_color(&text_rect, -1, -1, menubar_item->text, text_color);

	*offset_x += box_width; // prepare for next menu item to be drawn

	if (item_state == 1 && menubar_item->associated_menu != NULL) {
		draw_menu(menubar_item->associated_menu, box_left);
	}

};

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

void draw_menubar() {
	int scaled_width = 0, scaled_height = 0;
	SDL_GetRendererOutputSize(renderer_, &scaled_width, &scaled_height);
	if (scaled_width > 0 && scaled_height > 0) {
		float scale_x = 320.0f / scaled_width ;
		float scale_y = 200.0f / scaled_height;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		mouse_x = (int) ((float) mouse_x * scale_x + 0.5f);
		mouse_y = (int) ((float) mouse_y * scale_y + 0.5f);

		font_type* saved_font = textstate.ptr_font;
		textstate.ptr_font = &small_font;

		draw_rect_with_alpha(&menubar_rect, color_15_brightwhite, menu_alpha);
		int left = 0;
		draw_menubar_item(&game_menubar_item, &left);
		draw_menubar_item(&options_menubar_item, &left);
		draw_menubar_item(&capture_menubar_item, &left);
		if (cheats_enabled) {
			draw_menubar_item(&cheats_menubar_item, &left);
		}

		if (mouse_y < menubar_rect.bottom && mouse_x >= left) {
			selected_menu_id = -1;
		}

		textstate.ptr_font = saved_font;
	}
}

rect_type explanation_rect = {170, 20, 200, 300};
int highlighted_setting_id = SETTING_ENABLE_INFO_SCREEN;
int controlled_area = 0;
int next_setting_id = 0;
int previous_setting_id = 0;


void enter_settings_subsection(int settings_menu_id, setting_type* settings) {
	if (active_settings_menu_item != settings_menu_id) {
		highlighted_setting_id = settings[0].id;
	}
	active_settings_menu_item = settings_menu_id;
	if (!mouse_state_changed) highlighted_pause_menu_item = 0;
	controlled_area = 1;
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
			active_settings_menu_item = 0;
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
		case SETTING_ENABLE_LIGHTING:
			enable_lighting = 0;
			need_full_redraw = 1;
			break;
		case SETTING_ENABLE_SOUND:
			turn_sound_on_off(0);
			break;
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
	setting_box.left -= 10;
	setting_box.right += 10;

	if (mouse_clicked && is_mouse_over_rect(&setting_box)) {
		highlighted_setting_id = setting->id;
		controlled_area = 1;
	}

	if (highlighted_setting_id == setting->id) {
		next_setting_id = setting->next;
		previous_setting_id = setting->previous;
		SDL_Rect dest_rect;
		rect_to_sdlrect(&setting_box, &dest_rect);
		uint32_t rgb_color = SDL_MapRGBA(overlay_surface->format, 40, 40, 40, 240);
		if (SDL_FillRect(overlay_surface, &dest_rect, rgb_color) != 0) {
			sdlperror("SDL_FillRect");
			quit(1);
		}
//		draw_rect_with_alpha(&setting_box, color_8_darkgray, 200);
		draw_setting_explanation(setting);
	}

	show_text_with_color(&text_rect, -1, -1, setting->text, text_color);

	if (setting->style == SETTING_STYLE_TOGGLE) {
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
	}

	*y_offset += 15;
}

void draw_settings_area() {
	rect_type settings_area_rect = {0, 80, 170, 320};
	shrink2_rect(&settings_area_rect, &settings_area_rect, 20, 20);

	int y_offset = 0;
	if (active_settings_menu_item == SETTINGS_MENU_GENERAL) {
		for (int i = 0; i < COUNT(general_settings); ++i) {
			draw_setting(&general_settings[i], &settings_area_rect, &y_offset, color_15_brightwhite);
		}
	} else if (active_settings_menu_item == SETTINGS_MENU_VISUALS) {
		for (int i = 0; i < COUNT(visuals_settings); ++i) {
			draw_setting(&visuals_settings[i], &settings_area_rect, &y_offset, color_15_brightwhite);
		}
	} else if (active_settings_menu_item == SETTINGS_MENU_GAMEPLAY) {
		for (int i = 0; i < COUNT(gameplay_settings); ++i) {
			draw_setting(&gameplay_settings[i], &settings_area_rect, &y_offset, color_15_brightwhite);
		}
	} else if (active_settings_menu_item == SETTINGS_MENU_MODS) {
		for (int i = 0; i < COUNT(mods_settings); ++i) {
			draw_setting(&mods_settings[i], &settings_area_rect, &y_offset, color_15_brightwhite);
		}
	}

}



void draw_settings_menu() {
	pause_menu_alpha = 230;
	draw_rect_with_alpha(&screen_rect, color_0_black, pause_menu_alpha);
	rect_type pause_rect_outer = {0, 10, 192, 80};
	rect_type pause_rect_inner;
	shrink2_rect(&pause_rect_inner, &pause_rect_outer, 5, 5);

	if (!mouse_state_changed) {
		if (menu_control_y == 1) {
			play_sound(sound_21_loose_shake_2);
			play_next_sound();
			if (controlled_area == 0) {
				highlighted_pause_menu_item = next_pause_menu_item;
			} else if (controlled_area == 1) {
				highlighted_setting_id = next_setting_id;
			}
		} else if (menu_control_y == -1) {
			play_sound(sound_21_loose_shake_2);
			play_next_sound();
			if (controlled_area == 0) {
				highlighted_pause_menu_item = previous_pause_menu_item;
			} else if (controlled_area == 1) {
				highlighted_setting_id = previous_setting_id;
			}
		}
	}

//	highlighted_pause_menu_item = -1;
	int y_offset = 50;
	for (int i = 0; i < COUNT(settings_menu_items); ++i) {
		pause_menu_item_type* item = &settings_menu_items[i];
		int text_color = (active_settings_menu_item == item->id) ? color_15_brightwhite : color_7_lightgray;
		draw_pause_menu_item(&settings_menu_items[i], &pause_rect_inner, &y_offset, text_color);
	}

	draw_settings_area();
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
			return key;
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
			clicked_or_pressed_enter = 1;
			return 0;
		case SDL_SCANCODE_ESCAPE:
			if (drawn_menu == 1) {
				play_sound(sound_22_loose_shake_3);
				play_next_sound();
				if (controlled_area == 1) {
					controlled_area = 0;
					highlighted_pause_menu_item = active_settings_menu_item;
					active_settings_menu_item = 0;
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

#endif //USE_MENU
