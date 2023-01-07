/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2023  Dávid Nagy

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

// seg007:0000
void process_trobs() {
	word need_delete = 0;
	if (trobs_count == 0) return;
	for (word index = 0; index < trobs_count; ++index) {
		trob = trobs[index];
		animate_tile();
		trobs[index].type = trob.type;
		if (trob.type < 0) {
			need_delete = 1;
		}
	}
	if (need_delete) {
		word new_index;
		for (word index = new_index = 0; index < trobs_count; ++index) {
			if (trobs[index].type >= 0) {
				trobs[new_index] = trobs[index];
				new_index++;
			}
		}
		trobs_count = new_index;
	}
}

// seg007:00AF
void animate_tile() {
	get_room_address(trob.room);
	switch (get_curr_tile(trob.tilepos)) {
		case tiles_19_torch:
		case tiles_30_torch_with_debris:
			animate_torch();
		break;
		case tiles_6_closer:
		case tiles_15_opener:
			animate_button();
		break;
		case tiles_2_spike:
			animate_spike();
		break;
		case tiles_11_loose:
			animate_loose();
		break;
		case tiles_0_empty:
			animate_empty();
		break;
		case tiles_18_chomper:
			animate_chomper();
		break;
		case tiles_4_gate:
			animate_door();
		break;
		case tiles_16_level_door_left:
			animate_leveldoor();
		break;
		case tiles_10_potion:
			animate_potion();
		break;
		case tiles_22_sword:
			animate_sword();
		break;
		default:
			trob.type = -1;
		break;
	}
	curr_room_modif[trob.tilepos] = curr_modifier;
}

// seg007:0166
short is_trob_in_drawn_room() {
	if (trob.room != drawn_room) {
		trob.type = -1;
		return 0;
	} else {
		return 1;
	}
}

// seg007:017E
void set_redraw_anim_right() {
	set_redraw_anim(get_trob_right_pos_in_drawn_room(), 1);
}

// seg007:018C
void set_redraw_anim_curr() {
	set_redraw_anim(get_trob_pos_in_drawn_room(), 1);
}

// seg007:019A
void redraw_at_trob() {
	redraw_height = 63;
	word tilepos = get_trob_pos_in_drawn_room();
	set_redraw_full(tilepos, 1);
	set_wipe(tilepos, 1);
}

// seg007:01C5
void redraw_21h() {
	redraw_height = 0x21;
	redraw_tile_height();
}

// seg007:01D0
void redraw_11h() {
	redraw_height = 0x11;
	redraw_tile_height();
}

// seg007:01DB
void redraw_20h() {
	redraw_height = 0x20;
	redraw_tile_height();
}

// seg007:01E6
void draw_trob() {
	word tilepos = get_trob_right_pos_in_drawn_room();
	set_redraw_anim(tilepos, 1);
	set_redraw_fore(tilepos, 1);
	set_redraw_anim(get_trob_right_above_pos_in_drawn_room(), 1);
}

// seg007:0218
void redraw_tile_height() {
	short tilepos = get_trob_pos_in_drawn_room();
	set_redraw_full(tilepos, 1);
	set_wipe(tilepos, 1);
	tilepos = get_trob_right_pos_in_drawn_room();
	set_redraw_full(tilepos, 1);
	set_wipe(tilepos, 1);
}

// seg007:0258
short get_trob_pos_in_drawn_room() {
	short tilepos = trob.tilepos;
	if (trob.room == room_A) {
		if (tilepos >= 20 && tilepos < 30) {
			// 20..29 -> -1..-10
			tilepos = 19 - tilepos;
		} else {
			tilepos = 30;
		}
	} else {
		if (trob.room != drawn_room) {
			tilepos = 30;
		}
	}
	return tilepos;
}

// seg007:029D
short get_trob_right_pos_in_drawn_room() {
	word tilepos = trob.tilepos;
	if (trob.room == drawn_room) {
		if (tilepos % 10 != 9) {
			++tilepos;
		} else {
			tilepos = 30;
		}
	} else if (trob.room == room_L) {
		if (tilepos % 10 == 9) {
			tilepos -= 9;
		} else {
			tilepos = 30;
		}
	} else if (trob.room == room_A) {
		if (tilepos >= 20 && tilepos < 29) {
			// 20..28 -> -2..-10
			tilepos = 18 - tilepos;
		} else {
			tilepos = 30;
		}
	} else if (trob.room == room_AL && tilepos == 29) {
		tilepos = -1;
	} else {
		tilepos = 30;
	}
	return tilepos;
}

// seg007:032C
short get_trob_right_above_pos_in_drawn_room() {
	word tilepos = trob.tilepos;
	if (trob.room == drawn_room) {
		if (tilepos % 10 != 9) {
			if (tilepos < 10) {
				// 0..8 -> -2..-10
				tilepos = -(tilepos + 2);
			} else {
				tilepos -= 9;
			}
		} else {
			tilepos = 30;
		}
	} else if (trob.room == room_L) {
		if (tilepos == 9) {
			tilepos = -1;
		} else {
			if (tilepos % 10 == 9) {
				tilepos -= 19;
			} else {
				tilepos = 30;
			}
		}
	} else if (trob.room == room_B) {
		if (tilepos < 9) {
			tilepos += 21;
		} else {
			tilepos = 30;
		}
	} else if (trob.room == room_BL && tilepos == 9) {
		tilepos = 20;
	} else {
		tilepos = 30;
	}
	return tilepos;
}

// seg007:03CF
void animate_torch() {
	//if (is_trob_in_drawn_room()) {
	// Keep animating torches in the rightmost column of the left-side room as well, because they are visible in the current room.
	if (trob.room == drawn_room || (trob.room == room_L && (trob.tilepos % 10) == 9) ) {
		curr_modifier = get_torch_frame(curr_modifier);
		set_redraw_anim_right();
	} else {
		trob.type = -1;
	}
}

// seg007:03E9
void animate_potion() {
	if (trob.type >= 0 && is_trob_in_drawn_room()) {
		word type = curr_modifier & 0xF8;
		curr_modifier = bubble_next_frame(curr_modifier & 0x07) | type;
#ifdef FIX_LOOSE_NEXT_TO_POTION
		redraw_at_trob();
#else
		set_redraw_anim_curr();
#endif
	}
}

// seg007:0425
void animate_sword() {
	if (is_trob_in_drawn_room()) {
		--curr_modifier;
		if (curr_modifier == 0) {
			curr_modifier = (prandom(255) & 0x3F) + 0x28;
		}
#ifdef FIX_LOOSE_NEXT_TO_POTION
		redraw_at_trob();
#else
		set_redraw_anim_curr();
#endif
	}
}

// seg007:0448
void animate_chomper() {
	if (trob.type >= 0) {
		word blood = curr_modifier & 0x80;
		word frame = (curr_modifier & 0x7F) + 1;
		if (frame > /*15*/ custom->chomper_speed) {
			frame = 1;
		}
		curr_modifier = blood | frame;
		if (frame == 2) {
			play_sound(sound_47_chomper); // chomper
		}
		// If either:
		// - Kid left this room
		// - Kid left this row
		// - Kid died but not in this chomper
		// and chomper is past frame 6
		// then stop.
		if ((trob.room != drawn_room || trob.tilepos / 10 != Kid.curr_row ||
			(Kid.alive >= 0 && blood == 0)) && (curr_modifier & 0x7F) >= 6
		) {
			trob.type = -1;
		}
	}
	if ((curr_modifier & 0x7F) < 6) {
		redraw_at_trob();
	}
}

// seg007:04D3
void animate_spike() {
	if (trob.type >= 0) {
		// 0xFF means a disabled spike.
		if (curr_modifier == 0xFF) return;
		if (curr_modifier & 0x80) {
			--curr_modifier;
			if (curr_modifier & 0x7F) return;
			curr_modifier = 6;
		} else {
			++curr_modifier;
			if (curr_modifier == 5) {
				curr_modifier = 0x8F;
			} else if (curr_modifier == 9) {
				curr_modifier = 0;
				trob.type = -1;
			}
		}
	}
	redraw_21h();
}

// data:27B2
const byte gate_close_speeds[] = {0, 0, 0, 20, 40, 60, 80, 100, 120};
// data:27C0
const byte door_delta[] = {-1, 4, 4};
// seg007:0522
void animate_door() {
/*
Possible values of anim_type:
0: closing
1: open
2: permanent open
3,4,5,6,7,8: fast closing with speeds 20,40,60,80,100,120 /4 pixel/frame
*/
	sbyte anim_type = trob.type;
	if (anim_type >= 0) {
		if (anim_type >= 3) {
			// closing fast
			if (anim_type < 8) {
				++anim_type;
				trob.type = anim_type;
			}
			short new_mod = curr_modifier - gate_close_speeds[anim_type];
			curr_modifier = new_mod;
			//if ((sbyte)curr_modifier < 0) {
			if (new_mod < 0) {
			//if ((curr_modifier -= gate_close_speeds[anim_type]) < 0) {
				curr_modifier = 0;
				trob.type = -1;
				play_sound(sound_6_gate_closing_fast); // gate closing fast
			}
		} else {
			if (curr_modifier != 0xFF) {
				// 0xFF means permanently open.
				curr_modifier += door_delta[anim_type];
				if (anim_type == 0) {
					// closing
					if (curr_modifier != 0) {
						if (curr_modifier < 188) {
							if ((curr_modifier & 3) == 3) {
								play_door_sound_if_visible(sound_4_gate_closing); // gate closing
							}
						}
					} else {
						gate_stop();
					}
				} else {
					// opening
					if (curr_modifier < 188) {
						if ((curr_modifier & 7) == 0) {
							play_sound(sound_5_gate_opening); // gate opening
						}
					} else {
						// stop
						if (anim_type < 2) {
							// after regular open
							curr_modifier = 238;
							trob.type = 0; // closing
							play_sound(sound_7_gate_stop); // gate stop (after opening)
						} else {
							// after permanent open
							curr_modifier = 0xFF; // keep open
							gate_stop();
						}
					}
				}
			} else {
				gate_stop();
			}
		}
	}
	draw_trob();
}

// seg007:05E3
void gate_stop() {
	trob.type = -1;
	play_door_sound_if_visible(sound_7_gate_stop); // gate stop (after closing)
}

// data:27B8
const byte leveldoor_close_speeds[] = {0, 5, 17, 99, 0};
// seg007:05F1
void animate_leveldoor() {
/*
Possible values of trob_type:
0: open
1: open (with button)
2: open
3,4,5,6: fast closing with speeds 0,5,17,99 pixel/frame
*/
	word trob_type = trob.type;
	if (trob.type >= 0) {
		if (trob_type >= 3) {
			// closing
			++trob.type;
			curr_modifier -= leveldoor_close_speeds[trob.type - 3];
			if ((sbyte)curr_modifier < 0) {
				curr_modifier = 0;
				trob.type = -1;
				play_sound(sound_14_leveldoor_closing); // level door closing
			} else {
				if (trob.type == 4 &&
					(sound_flags & sfDigi)
				) {
					sound_interruptible[sound_15_leveldoor_sliding] = 1;
					play_sound(sound_15_leveldoor_sliding); // level door sliding (closing)
				}
			}
		} else {
			// opening
			++curr_modifier;
			if (curr_modifier >= 43) {
				trob.type = -1;
#ifdef FIX_FEATHER_INTERRUPTED_BY_LEVELDOOR
				if (!(fixes->fix_feather_interrupted_by_leveldoor && is_feather_fall))
#endif
				stop_sounds();
				if (leveldoor_open == 0 || leveldoor_open == 2) {
					leveldoor_open = 1;
					if (current_level == /*4*/ custom->mirror_level) {
						// Special event: place mirror
						get_tile(/*4*/ custom->mirror_room, /*4*/ custom->mirror_column, /*0*/ custom->mirror_row);
						curr_room_tiles[curr_tilepos] = /*tiles_13_mirror*/ custom->mirror_tile;
					}
				}
			} else {
				sound_interruptible[sound_15_leveldoor_sliding] = 0;
				play_sound(sound_15_leveldoor_sliding); // level door sliding (opening)
			}
		}
	}
	set_redraw_anim_right();
}

// seg007:06AD
short bubble_next_frame(short curr) {
	short next = curr + 1;
	if (next >= 8) next = 1;
	return next;
}

// seg007:06CD
short get_torch_frame(short curr) {
	short next = prandom(255);
	if (next != curr) {
		if (next < 9) {
			return next;
		} else {
			next = curr;
		}
	}
	++next;
	if (next >= 9) next = 0;
	return next;
}

// seg007:070A
void set_redraw_anim(short tilepos, byte frames) {
	if (tilepos < 30) {
		if (tilepos < 0) {
			++tilepos;
			redraw_frames_above[-tilepos] = frames;
			// or simply: ~tilepos
		} else {
			redraw_frames_anim[tilepos] = frames;
		}
	}
}

// seg007:0738
void set_redraw2(short tilepos, byte frames) {
	if (tilepos < 30) {
		if (tilepos < 0) {
			// trying to draw a mob at a negative tilepos, in the range -1 .. -10
			// used e.g. when the kid is climbing up to the room above
			// however, loose tiles falling out of the room end up with a negative tilepos {-2 .. -11} !
			tilepos = (-tilepos) - 1;
			if (tilepos > 9) tilepos = 9; // prevent array index out of bounds!
			redraw_frames_above[tilepos] = frames;
		} else {
			redraw_frames2[tilepos] = frames;
		}
	}
}

// seg007:0766
void set_redraw_floor_overlay(short tilepos, byte frames) {
	if (tilepos < 30) {
		if (tilepos < 0) {
			++tilepos;
			redraw_frames_above[-tilepos] = frames;
			// or simply: ~tilepos
		} else {
			redraw_frames_floor_overlay[tilepos] = frames;
		}
	}
}

// seg007:0794
void set_redraw_full(short tilepos, byte frames) {
	if (tilepos < 30) {
		if (tilepos < 0) {
			++tilepos;
			redraw_frames_above[-tilepos] = frames;
			// or simply: ~tilepos
		} else {
			redraw_frames_full[tilepos] = frames;
		}
	}
}

// seg007:07C2
void set_redraw_fore(short tilepos, byte frames) {
	if (tilepos < 30 && tilepos >= 0) {
		redraw_frames_fore[tilepos] = frames;
	}
}

// seg007:07DF
void set_wipe(short tilepos, byte frames) {
	if (tilepos < 30 && tilepos >= 0) {
		if (wipe_frames[tilepos] != 0) {
			redraw_height = MAX(wipe_heights[tilepos], redraw_height);
		}
		wipe_heights[tilepos] = redraw_height;
		wipe_frames[tilepos] = frames;
	}
}

// seg007:081E
void start_anim_torch(short room,short tilepos) {
	curr_room_modif[tilepos] = prandom(8);
	add_trob(room, tilepos, 1);
}

// seg007:0847
void start_anim_potion(short room,short tilepos) {
	curr_room_modif[tilepos] &= 0xF8;
	curr_room_modif[tilepos] |= prandom(6) + 1;
	add_trob(room, tilepos, 1);
}

// seg007:087C
void start_anim_sword(short room,short tilepos) {
	curr_room_modif[tilepos] = prandom(0xFF) & 0x1F;
	add_trob(room, tilepos, 1);
}

// seg007:08A7
void start_anim_chomper(short room,short tilepos, byte modifier) {
	short old_modifier = curr_room_modif[tilepos];
	if (old_modifier == 0 || old_modifier >= 6) {
		curr_room_modif[tilepos] = modifier;
		add_trob(room, tilepos, 1);
	}
}

// seg007:08E3
void start_anim_spike(short room,short tilepos) {
	sbyte old_modifier = curr_room_modif[tilepos];
	if (old_modifier <= 0) {
		if (old_modifier == 0) {
			add_trob(room, tilepos, 1);
			play_sound(sound_49_spikes); // spikes
		} else {
			// 0xFF means a disabled spike.
			if (old_modifier != (sbyte)0xFF) {
				curr_room_modif[tilepos] = 0x8F;
			}
		}
	}
}

// seg007:092C
short trigger_gate(short room,short tilepos,short button_type) {
	byte modifier = curr_room_modif[tilepos];
	if (button_type == tiles_15_opener) {
		// If the gate is permanently open, don't to anything.
		if (modifier == 0xFF) return -1;
		if (modifier >= 188) { // if it's already open
			curr_room_modif[tilepos] = 238; // keep it open for a while
			return -1;
		}
		curr_room_modif[tilepos] = (modifier + 3) & 0xFC;
		return 1; // regular open
	} else if (button_type == tiles_14_debris) {
		// If it's not fully open:
		if (modifier < 188) return 2; // permanent open
		curr_room_modif[tilepos] = 0xFF; // keep open
		return -1;
	} else {
		if (modifier != 0) {
			return 3; // close fast
		} else {
			// already closed
			return -1;
		}
	}
}

// seg007:0999
short trigger_1(short target_type,short room,short tilepos,short button_type) {
	short result = -1;
	if (target_type == tiles_4_gate) {
		result = trigger_gate(room, tilepos, button_type);
	} else if (target_type == tiles_16_level_door_left) {
		if (curr_room_modif[tilepos] != 0) {
			result = -1;
		} else {
			result = 1;
		}
	} else if (custom->allow_triggering_any_tile) { //allow_triggering_any_tile hack
		result = 1;
	}
	return result;
}

// seg007:09E5
void do_trigger_list(short index,short button_type) {
	word room;
	word tilepos;
	byte target_type;
	sbyte trigger_result;
//	while (doorlink1_ad[index] != -1) { // these can't be equal!
	while (1) {  // Same as the above but just a little faster and no compiler warning.
		room = get_doorlink_room(index);
		get_room_address(room);
		tilepos = get_doorlink_tile(index);
		target_type = curr_room_tiles[tilepos] & 0x1F;
		trigger_result = trigger_1(target_type, room, tilepos, button_type);
		if (trigger_result >= 0) {
			add_trob(room, tilepos, trigger_result);
		}
		if (get_doorlink_next(index) == 0) break;
		index++;
	}
}

// seg007:0A5A
void add_trob(byte room,byte tilepos,sbyte type) {
	if (trobs_count >= TROBS_MAX) {
		show_dialog("Trobs Overflow");
		return /*0*/; // added
	}
	trob.room = room;
	trob.tilepos = tilepos;
	trob.type = type;
	short found = find_trob();
	if (found == -1) {
		// add new
		if (trobs_count == TROBS_MAX) return;
		trobs[trobs_count] = trob;
		trobs_count++;
	} else {
		// change existing
		trobs[found].type = trob.type;
	}
}

// seg007:0ACA
short find_trob() {
	for (short index = 0; index < trobs_count; ++index) {
		if (trobs[index].tilepos == trob.tilepos &&
			trobs[index].room == trob.room) return index;
	}
	return -1;
}

// seg007:0B0A
void clear_tile_wipes() {
	memset(redraw_frames_full, 0, sizeof(redraw_frames_full));
	memset(wipe_frames, 0, sizeof(wipe_frames));
	memset(wipe_heights, 0, sizeof(wipe_heights));
	memset(redraw_frames_anim, 0, sizeof(redraw_frames_anim));
	memset(redraw_frames_fore, 0, sizeof(redraw_frames_fore));
	memset(redraw_frames2, 0, sizeof(redraw_frames2));
	memset(redraw_frames_floor_overlay, 0, sizeof(redraw_frames_floor_overlay));
	memset(tile_object_redraw, 0, sizeof(tile_object_redraw));
	memset(redraw_frames_above, 0, sizeof(redraw_frames_above));
}

// seg007:0BB6
short get_doorlink_timer(short index) {
	return doorlink2_ad[index] & 0x1F;
}

// seg007:0BCD
short set_doorlink_timer(short index,byte value) {
	doorlink2_ad[index] &= 0xE0;
	doorlink2_ad[index] |= value & 0x1F;
	return doorlink2_ad[index];
}

// seg007:0BF2
short get_doorlink_tile(short index) {
	return doorlink1_ad[index] & 0x1F;
}

// seg007:0C09
short get_doorlink_next(short index) {
	return !(doorlink1_ad[index] & 0x80);
}

// seg007:0C26
short get_doorlink_room(short index) {
	return
		((doorlink1_ad[index] & 0x60) >> 5) +
		((doorlink2_ad[index] & 0xE0) >> 3);
}

// seg007:0C53
void trigger_button(int playsound,int button_type,int modifier) {
	get_curr_tile(curr_tilepos);
	if (button_type == 0) {
		// 0 means currently selected
		button_type = curr_tile;
	}
	if (modifier == -1) {
		// -1 means currently selected
		modifier = curr_modifier;
	}
	sbyte link_timer = get_doorlink_timer(modifier);
	// is the event jammed?
	if (link_timer != 0x1F) {
		set_doorlink_timer(modifier, 5);
		if (link_timer < 2) {
			add_trob(curr_room, curr_tilepos, 1);
			redraw_11h();
			is_guard_notice = 1;
			if (playsound) {
				play_sound(sound_3_button_pressed); // button pressed
			}
		}
		do_trigger_list(modifier, button_type);
	}
}

// seg007:0CD9
void died_on_button() {
	word button_type = get_curr_tile(curr_tilepos);
	word modifier = curr_modifier;
	if (curr_tile == tiles_15_opener) {
		curr_room_tiles[curr_tilepos] = tiles_1_floor;
		curr_room_modif[curr_tilepos] = 0;
		button_type = tiles_14_debris; // force permanent open
	} else {
		curr_room_tiles[curr_tilepos] = tiles_5_stuck;
	}
	trigger_button(1, button_type, modifier);
}

// seg007:0D3A
void animate_button() {
	if (trob.type >= 0) {
		word timer = get_doorlink_timer(curr_modifier) - 1;
		set_doorlink_timer(curr_modifier, timer);
		if (timer < 2) {
			trob.type = -1;
			redraw_11h();
		}
	}
}

// seg007:0D72
void start_level_door(short room,short tilepos) {
	curr_room_modif[tilepos] = 43; // start fully open
	add_trob(room, tilepos, 3);
}

// seg007:0D93
void animate_empty() {
	trob.type = -1;
	redraw_20h();
}

// data:2284
const word y_loose_land[] = {2, 65, 128, 191, 254};
// seg007:0D9D
void animate_loose() {
	short anim_type = trob.type;
	if (anim_type >= 0) {
		++curr_modifier;
		if (curr_modifier & 0x80) {
			// just shaking
			// don't stop on level 13, needed for the auto-falling floors
			if (current_level == /*13*/ custom->loose_tiles_level) return;
			if (curr_modifier >= 0x84) {
				curr_modifier = 0;
				trob.type = -1;
			}
			loose_shake(!curr_modifier);
		} else {
			// something is on the floor
			// should it fall already?
			if (curr_modifier >= /*11*/ custom->loose_floor_delay) {
                word room = trob.room;
                word tilepos = trob.tilepos;
#ifdef FIX_DROP_2_ROOMS_CLIMBING_LOOSE_TILE
                if (fixes->fix_drop_2_rooms_climbing_loose_tile &&
                        room == level.roomlinks[Kid.room - 1].up && // the tile is in the room above
                        tilepos / 10 == 2 && // at row 2
                        Kid.curr_row == 0 && // prince is at a row 0 of the room below
                        Kid.curr_col == tilepos % 10 && // and at the same column
                        Kid.frame >= frame_135_climbing_1 && // and is climbing
                        Kid.frame < frame_141_climbing_7) { // prince's row gets changed in the sequence before the frame 141
                    loose_shake(0);
                } else {
#endif
                    curr_modifier = remove_loose(room, tilepos);
                    trob.type = -1;
                    curmob.xh = (tilepos % 10) << 2;
                    word row = tilepos / 10;
                    curmob.y = y_loose_land[row + 1];
                    curmob.room = room;
                    curmob.speed = 0;
                    curmob.type = 0;
                    curmob.row = row;
                    add_mob();
#ifdef FIX_DROP_2_ROOMS_CLIMBING_LOOSE_TILE
                }
#endif
			} else {
				loose_shake(0);
			}
		}
	}
	redraw_20h();
}

// data:2734
const byte loose_sound[] = {0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0};
// seg007:0E55
void loose_shake(int arg_0) {
	word sound_id;
	if (arg_0 || loose_sound[curr_modifier & 0x7F]) {
		do {
			// Sounds 20,21,22: loose floor shaking
			sound_id = prandom(2) + sound_20_loose_shake_1;
		} while(sound_id == last_loose_sound);

#ifdef USE_REPLAY
		// Skip this prandom call if we are replaying, and the replay file was made with an old version of SDLPoP (which didn't have this call).
		if (!(replaying && g_deprecation_number < 2))
#endif
		{
			prandom(2); // For vanilla pop compatibility, an RNG cycle is wasted here
			// Note: In DOS PoP, it's wasted a few lines below.
		}

		if (sound_flags & sfDigi) {
			last_loose_sound = sound_id;
			// random sample rate (10500..11500)
			//sound_pointers[sound_id]->samplerate = prandom(1000) + 10500;
		}
		play_sound(sound_id);
	}
}

// seg007:0EB8
int remove_loose(int room, int tilepos) {
	curr_room_tiles[tilepos] = tiles_0_empty;
	// note: the level type is used to determine the modifier of the empty space left behind
	return custom->tbl_level_type[current_level];
}

// seg007:0ED5
void make_loose_fall(byte modifier) {
	// is it a "solid" loose floor?
	if ((curr_room_tiles[curr_tilepos] & 0x20) == 0) {
		if ((sbyte)curr_room_modif[curr_tilepos] <= 0) {
			curr_room_modif[curr_tilepos] = modifier;
			add_trob(curr_room, curr_tilepos, 0);
			redraw_20h();
		}
	}
}

// seg007:0F13
void start_chompers() {
	short timing = 15;
	if ((byte)Char.curr_row < 3) {
		get_room_address(Char.room);
		for (short column = 0, tilepos = tbl_line[Char.curr_row];
			column < 10; ++column, ++tilepos
		){
			if (get_curr_tile(tilepos) == tiles_18_chomper) {
				short modifier = curr_modifier & 0x7F;
				if (modifier == 0 || modifier >= 6) {
					start_anim_chomper(Char.room, tilepos, timing | (curr_modifier & 0x80));
					timing = next_chomper_timing(timing);
				}
			}
		}
	}
}

// seg007:0F9A
int next_chomper_timing(byte timing) {
	// 15,12,9,6,13,10,7,14,11,8,repeat
	timing -= 3;
	if (timing < 6) {
		timing += 10;
	}
	return timing;
}

// seg007:0FB4
void loose_make_shake() {
	// don't shake on level 13
	if (curr_room_modif[curr_tilepos] == 0 && current_level != /*13*/ custom->loose_tiles_level) {
		curr_room_modif[curr_tilepos] = 0x80;
		add_trob(curr_room, curr_tilepos, 1);
	}
}

// seg007:0FE0
void do_knock(int room,int tile_row) {
	for (short tile_col = 0; tile_col < 10; ++tile_col) {
		if (get_tile(room, tile_col, tile_row) == tiles_11_loose) {
			loose_make_shake();
		}
	}
}

// seg007:1010
void add_mob() {
	if (mobs_count >= 14) {
		show_dialog("Mobs Overflow");
		return /*0*/; // added
	}
	mobs[mobs_count] = curmob;
	mobs_count++;
}

// seg007:1041
short get_curr_tile(short tilepos) {
	curr_modifier = curr_room_modif[tilepos];
	return curr_tile = curr_room_tiles[tilepos] & 0x1F;
}

// data:43DC
word curmob_index;

// seg007:1063
void do_mobs() {
	short n_mobs = mobs_count;
	for (curmob_index = 0; n_mobs > curmob_index; ++curmob_index) {
		curmob = mobs[curmob_index];
		move_mob();
		check_loose_fall_on_kid();
		mobs[curmob_index] = curmob;
	}
	short new_index = 0;
	for (short index = 0; index < mobs_count; ++index) {
		if (mobs[index].speed != -1) {
			mobs[new_index] = mobs[index];
			new_index++;
		}
	}
	mobs_count = new_index;
}

// seg007:110F
void move_mob() {
	if (curmob.type == 0) {
		move_loose();
	}
	if (curmob.speed <= 0) {
		++curmob.speed;
	}
}

// data:227A
const short y_something[] = {-1, 62, 125, 188, 25};
// data:594A
word curr_tile_temp;
// seg007:1126
void move_loose() {
	if (curmob.speed < 0) return;
	if (curmob.speed < 29) curmob.speed += 3;
	curmob.y += curmob.speed;
	if (curmob.room == 0) {
		if (curmob.y < 210) {
			return;
		} else {
			curmob.speed = -2;
			return;
		}
	}
	if (curmob.y < 226 && y_something[curmob.row + 1] <= curmob.y) {
		// fell into a different row
		curr_tile_temp = get_tile(curmob.room, curmob.xh >> 2, curmob.row);
		if (curr_tile_temp == tiles_11_loose) {
			loose_fall();
		}
		if (curr_tile_temp == tiles_0_empty ||
			curr_tile_temp == tiles_11_loose
		) {
			mob_down_a_row();
			return;
		}
		play_sound(sound_2_tile_crashing); // tile crashing
		do_knock(curmob.room, curmob.row);
		curmob.y = y_something[curmob.row + 1];
		curmob.speed = -2;
		loose_land();
	}
}

// seg007:11E8
void loose_land() {
	short button_type = 0;
	short tiletype = get_tile(curmob.room, curmob.xh >> 2, curmob.row);
	switch (tiletype) {
		case tiles_15_opener:
			curr_room_tiles[curr_tilepos] = tiles_14_debris;
			button_type = tiles_14_debris;
		// fallthrough!
		case tiles_6_closer:
			trigger_button(1, button_type, -1);
			tiletype = get_tile(curmob.room, curmob.xh >> 2, curmob.row);
		// fallthrough!
		case tiles_1_floor:
		case tiles_2_spike:
		case tiles_10_potion:
		case tiles_19_torch:
		case tiles_30_torch_with_debris:
			if (tiletype == tiles_19_torch ||
				tiletype == tiles_30_torch_with_debris
			) {
				curr_room_tiles[curr_tilepos] = tiles_30_torch_with_debris;
			} else {
				curr_room_tiles[curr_tilepos] = tiles_14_debris;
			}
			redraw_at_cur_mob();
			if (tile_col != 0) {
				set_redraw_full(curr_tilepos - 1, 1);
			}
	}
}

// seg007:12CB
void loose_fall() {
	curr_room_modif[curr_tilepos] = remove_loose(curr_room, curr_tilepos);
	curmob.speed >>= 1;
	mobs[curmob_index] = curmob;
	curmob.y += 6;
	mob_down_a_row();
	add_mob();
	curmob = mobs[curmob_index];
	redraw_at_cur_mob();
}

// seg007:132C
void redraw_at_cur_mob() {
	if (curmob.room == drawn_room) {
		redraw_height = 0x20;
		set_redraw_full(curr_tilepos, 1);
		set_wipe(curr_tilepos, 1);
		// Redraw tile to the right only if it's in the same room.
		if ((curr_tilepos % 10) + 1 < 10) { // changed
			set_redraw_full(curr_tilepos + 1, 1);
			set_wipe(curr_tilepos + 1, 1);
		}
	}
}

// seg007:1387
void mob_down_a_row() {
	++curmob.row;
	if (curmob.row >= 3) {
		curmob.y -= 192;
		curmob.row = 0;
		curmob.room = level.roomlinks[curmob.room - 1].down;
	}
}

// seg007:13AE
void draw_mobs() {
	for (short index = 0; index < mobs_count; ++index) {
		curmob = mobs[index];
		draw_mob();
	}
}

// seg007:13E5
void draw_mob() {
	short ypos = curmob.y;
	if (curmob.room == drawn_room) {
		if (curmob.y >= 210) return;
	} else if (curmob.room == room_B) {
		if (ABS((sbyte)ypos) >= 18) return;
		curmob.y += 192;
		ypos = curmob.y;
	} else if (curmob.room == room_A) {
		if (curmob.y < 174) return;
		ypos = curmob.y - 189;
	} else {
		return;
	}
	short tile_col = curmob.xh >> 2;
	short tile_row = y_to_row_mod4(ypos);
	obj_tilepos = get_tilepos_nominus(tile_col, tile_row);
	++tile_col;
	short tilepos = get_tilepos(tile_col, tile_row);
	set_redraw2(tilepos, 1);
	set_redraw_fore(tilepos, 1);
	short top_row = y_to_row_mod4(ypos - 18);
	if (top_row != tile_row) {
		tilepos = get_tilepos(tile_col, top_row);
		set_redraw2(tilepos, 1);
		set_redraw_fore(tilepos, 1);
	}
	add_mob_to_objtable(ypos);
}

// seg007:14DE
void add_mob_to_objtable(int ypos) {
	objtable_type* curr_obj;
	word index = objtable_count++;
	curr_obj = &objtable[index];
	curr_obj->obj_type = curmob.type | 0x80;
	curr_obj->xh = curmob.xh;
	curr_obj->xl = 0;
	curr_obj->y = ypos;
	curr_obj->chtab_id = id_chtab_6_environment;
	curr_obj->id = 10;
	curr_obj->clip.top = 0;
	curr_obj->clip.left = 0;
	curr_obj->clip.right = 40;
	mark_obj_tile_redraw(index);
}

// seg007:153E
void sub_9A8E() {
	// This function is not used.
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_top, &rect_top, 0);
}

// seg007:1556
int is_spike_harmful() {
	sbyte modifier = curr_room_modif[curr_tilepos];
	if (modifier == 0 || modifier == -1) {
		return 0;
	} else if (modifier < 0) {
		return 1;
	} else if (modifier < 5) {
		return 2;
	} else {
		return 0;
	}
}

// seg007:1591
void check_loose_fall_on_kid() {
	loadkid();
	if (Char.room == curmob.room &&
		Char.curr_col == curmob.xh >> 2 &&
		curmob.y < Char.y &&
		Char.y - 30 < curmob.y
	) {
		fell_on_your_head();
		savekid();
	}
}

// seg007:15D3
void fell_on_your_head() {
	short frame = Char.frame;
	short action = Char.action;
	// loose floors hurt you in frames 5..14 (running) only on level 13
	if (
		(current_level == /*13*/ custom->loose_tiles_level || (frame < frame_5_start_run || frame >= 15)) &&
		(action < actions_2_hang_climb || action == actions_7_turn)
	) {
		Char.y = y_land[Char.curr_row + 1];
		if (take_hp(1)) {
			seqtbl_offset_char(seq_22_crushed); // dead (because of loose floor)
			if (frame == frame_177_spiked) { // spiked
				Char.x = char_dx_forward(-12);
			}
		} else {
			if (frame != frame_109_crouch) { // crouching
				if (get_tile_behind_char() == 0) {
					Char.x = char_dx_forward(-2);
				}
				seqtbl_offset_char(seq_52_loose_floor_fell_on_kid); // loose floor fell on Kid
			}
		}
	}
}

// seg007:1669
void play_door_sound_if_visible(int sound_id) {
	word tilepos = trob.tilepos;
	word gate_room = trob.room;
	word has_sound = 0;

#ifdef FIX_GATE_SOUNDS
	sbyte has_sound_condition;
	if (fixes->fix_gate_sounds)
		has_sound_condition =   (gate_room == room_L && tilepos % 10 == 9) ||
		                        (gate_room == drawn_room && tilepos % 10 != 9);
	else
		has_sound_condition =  gate_room == room_L ? tilepos % 10 == 9 :
		                      (gate_room == drawn_room && tilepos % 10 != 9);
	#define GATE_SOUND_CONDITION has_sound_condition
#else
	#define GATE_SOUND_CONDITION gate_room == room_L ? tilepos % 10 == 9 :          \
	                            (gate_room == drawn_room && tilepos % 10 != 9)
#endif
	// Special event: sound of closing gates
	if ((current_level == 3 && gate_room == 2) || GATE_SOUND_CONDITION) {
		has_sound = 1;
	}
	if (has_sound) {
		play_sound(sound_id);
	}
}
