/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2018  Dávid Nagy

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

// seg007:0000
void __pascal far process_trobs() {
	word need_delete;
	word index;
	word new_index;
	need_delete = 0;
	if (trobs_count == 0) return;
	for (index = 0; index < trobs_count; ++index) {
		trob = trobs[index];
		animate_tile();
		trobs[index].type = trob.type;
		if (trob.type < 0) {
			need_delete = 1;
		}
	}
	if (need_delete) {
		for (index = new_index = 0; index < trobs_count; ++index) {
			if (trobs[index].type >= 0) {
				trobs[new_index++] = trobs[index];
			}
		}
		trobs_count = new_index;
	}
}

// seg007:00AF
void __pascal far animate_tile() {
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
short __pascal far is_trob_in_drawn_room() {
	if (trob.room != drawn_room) {
		trob.type = -1;
		return 0;
	} else {
		return 1;
	}
}

// seg007:017E
void __pascal far set_redraw_anim_right() {
	set_redraw_anim(get_trob_right_pos_in_drawn_room(), 1);
}

// seg007:018C
void __pascal far set_redraw_anim_curr() {
	set_redraw_anim(get_trob_pos_in_drawn_room(), 1);
}

// seg007:019A
void __pascal far redraw_at_trob() {
	word tilepos;
	redraw_height = 63;
	tilepos = get_trob_pos_in_drawn_room();
	set_redraw_full(tilepos, 1);
	set_wipe(tilepos, 1);
}

// seg007:01C5
void __pascal far redraw_21h() {
	redraw_height = 0x21;
	redraw_tile_height();
}

// seg007:01D0
void __pascal far redraw_11h() {
	redraw_height = 0x11;
	redraw_tile_height();
}

// seg007:01DB
void __pascal far redraw_20h() {
	redraw_height = 0x20;
	redraw_tile_height();
}

// seg007:01E6
void __pascal far draw_trob() {
	word var_2;
	var_2 = get_trob_right_pos_in_drawn_room();
	set_redraw_anim(var_2, 1);
	set_redraw_fore(var_2, 1);
	set_redraw_anim(get_trob_right_above_pos_in_drawn_room(), 1);
}

// seg007:0218
void __pascal far redraw_tile_height() {
	short tilepos;
	tilepos = get_trob_pos_in_drawn_room();
	set_redraw_full(tilepos, 1);
	set_wipe(tilepos, 1);
	tilepos = get_trob_right_pos_in_drawn_room();
	set_redraw_full(tilepos, 1);
	set_wipe(tilepos, 1);
}

// seg007:0258
short __pascal far get_trob_pos_in_drawn_room() {
	short tilepos;
	tilepos = trob.tilepos;
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
short __pascal far get_trob_right_pos_in_drawn_room() {
	word tilepos;
	tilepos = trob.tilepos;
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
short __pascal far get_trob_right_above_pos_in_drawn_room() {
	word tilepos;
	tilepos = trob.tilepos;
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
void __pascal far animate_torch() {
	if (is_trob_in_drawn_room()) {
		curr_modifier = get_torch_frame(curr_modifier);
		set_redraw_anim_right();
	}
}

// seg007:03E9
void __pascal far animate_potion() {
	word type;
	if (trob.type >= 0 && is_trob_in_drawn_room()) {
		type = curr_modifier & 0xF8;
		curr_modifier = bubble_next_frame(curr_modifier & 0x07) | type;
		set_redraw_anim_curr();
	}
}

// seg007:0425
void __pascal far animate_sword() {
	if (is_trob_in_drawn_room()) {
		--curr_modifier;
		if (curr_modifier == 0) {
			curr_modifier = (prandom(255) & 0x3F) + 0x28;
		}
		set_redraw_anim_curr();
	}
}

// seg007:0448
void __pascal far animate_chomper() {
	word blood;
	word frame;
	if (trob.type >= 0) {
		blood = curr_modifier & 0x80;
		frame = (curr_modifier & 0x7F) + 1;
		if (frame > 15) {
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
void __pascal far animate_spike() {
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
void __pascal far animate_door() {
/*
Possible values of anim_type:
0: closing
1: open
2: permanent open
3,4,5,6,7,8: fast closing with speeds 20,40,60,80,100,120 /4 pixel/frame
*/
	sbyte anim_type;
	anim_type = trob.type;
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
void __pascal far gate_stop() {
	trob.type = -1;
	play_door_sound_if_visible(sound_7_gate_stop); // gate stop (after closing)
}

// data:27B8
const byte leveldoor_close_speeds[] = {0, 5, 17, 99, 0};
// seg007:05F1
void __pascal far animate_leveldoor() {
/*
Possible values of trob_type:
0: open
1: open (with button)
2: open
3,4,5,6: fast closing with speeds 0,5,17,99 pixel/frame
*/
	word trob_type;
	trob_type = trob.type;
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
				if (!(fix_feather_interrupted_by_leveldoor && is_feather_fall))
#endif
				stop_sounds();
				if (leveldoor_open == 0 || leveldoor_open == 2) {
					leveldoor_open = 1;
					if (current_level == 4) {
						// Special event: place mirror
						get_tile(4, 4, 0);
						curr_room_tiles[curr_tilepos] = tiles_13_mirror;
					}
				}
			} else {
				sound_interruptible[15] = 0;
				play_sound(sound_15_leveldoor_sliding); // level door sliding (opening)
			}
		}
	}
	set_redraw_anim_right();
}

// seg007:06AD
short __pascal far bubble_next_frame(short curr) {
	short next;
	next = curr + 1;
	if (next >= 8) next = 1;
	return next;
}

// seg007:06CD
short __pascal far get_torch_frame(short curr) {
	short next;
	next = prandom(255);
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
void __pascal far set_redraw_anim(short tilepos, byte frames) {
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
void __pascal far set_redraw2(short tilepos, byte frames) {
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
void __pascal far set_redraw_floor_overlay(short tilepos, byte frames) {
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
void __pascal far set_redraw_full(short tilepos, byte frames) {
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
void __pascal far set_redraw_fore(short tilepos, byte frames) {
	if (tilepos < 30 && tilepos >= 0) {
		redraw_frames_fore[tilepos] = frames;
	}
}

// seg007:07DF
void __pascal far set_wipe(short tilepos, byte frames) {
	if (tilepos < 30 && tilepos >= 0) {
		if (wipe_frames[tilepos] != 0) {
			redraw_height = MAX(wipe_heights[tilepos], redraw_height);
		}
		wipe_heights[tilepos] = redraw_height;
		wipe_frames[tilepos] = frames;
	}
}

// seg007:081E
void __pascal far start_anim_torch(short room,short tilepos) {
	curr_room_modif[tilepos] = prandom(8);
	add_trob(room, tilepos, 1);
}

// seg007:0847
void __pascal far start_anim_potion(short room,short tilepos) {
	curr_room_modif[tilepos] &= 0xF8;
	curr_room_modif[tilepos] |= prandom(6) + 1;
	add_trob(room, tilepos, 1);
}

// seg007:087C
void __pascal far start_anim_sword(short room,short tilepos) {
	curr_room_modif[tilepos] = prandom(0xFF) & 0x1F;
	add_trob(room, tilepos, 1);
}

// seg007:08A7
void __pascal far start_anim_chomper(short room,short tilepos, byte modifier) {
	short old_modifier;
	old_modifier = curr_room_modif[tilepos];
	if (old_modifier == 0 || old_modifier >= 6) {
		curr_room_modif[tilepos] = modifier;
		add_trob(room, tilepos, 1);
	}
}

// seg007:08E3
void __pascal far start_anim_spike(short room,short tilepos) {
	sbyte old_modifier;
	old_modifier = curr_room_modif[tilepos];
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
short __pascal far trigger_gate(short room,short tilepos,short button_type) {
	byte modifier;
	modifier = curr_room_modif[tilepos];
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
short __pascal far trigger_1(short target_type,short room,short tilepos,short button_type) {
	short result;
	result = -1;
	if (target_type == tiles_4_gate) {
		result = trigger_gate(room, tilepos, button_type);
	} else if (target_type == tiles_16_level_door_left) {
		if (curr_room_modif[tilepos] != 0) {
			result = -1;
		} else {
			result = 1;
		}
	} else if (allow_triggering_any_tile) { //allow_triggering_any_tile hack
		result = 1;
	}
	return result;
}

// seg007:09E5
void __pascal far do_trigger_list(short index,short button_type) {
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
		if (get_doorlink_next(index++) == 0) break;
	}
}

// seg007:0A5A
void __pascal far add_trob(byte room,byte tilepos,sbyte type) {
	short found;
	if (trobs_count >= 30) {
		show_dialog("Trobs Overflow");
		return /*0*/; // added
	}
	trob.room = room;
	trob.tilepos = tilepos;
	trob.type = type;
	found = find_trob();
	if (found == -1) {
		// add new
		if (trobs_count == 30) return;
		trobs[trobs_count++] = trob;
	} else {
		// change existing
		trobs[found].type = trob.type;
	}
}

// seg007:0ACA
short __pascal far find_trob() {
	short index;
	for (index = 0; index < trobs_count; ++index) {
		if (trobs[index].tilepos == trob.tilepos &&
			trobs[index].room == trob.room) return index;
	}
	return -1;
}

// seg007:0B0A
void __pascal far clear_tile_wipes() {
	memset_near(redraw_frames_full, 0, sizeof(redraw_frames_full));
	memset_near(wipe_frames, 0, sizeof(wipe_frames));
	memset_near(wipe_heights, 0, sizeof(wipe_heights));
	memset_near(redraw_frames_anim, 0, sizeof(redraw_frames_anim));
	memset_near(redraw_frames_fore, 0, sizeof(redraw_frames_fore));
	memset_near(redraw_frames2, 0, sizeof(redraw_frames2));
	memset_near(redraw_frames_floor_overlay, 0, sizeof(redraw_frames_floor_overlay));
	memset_near(tile_object_redraw, 0, sizeof(tile_object_redraw));
	memset_near(redraw_frames_above, 0, sizeof(redraw_frames_above));
}

// seg007:0BB6
short __pascal far get_doorlink_timer(short index) {
	return doorlink2_ad[index] & 0x1F;
}

// seg007:0BCD
short __pascal far set_doorlink_timer(short index,byte value) {
	doorlink2_ad[index] &= 0xE0;
	doorlink2_ad[index] |= value & 0x1F;
	return doorlink2_ad[index];
}

// seg007:0BF2
short __pascal far get_doorlink_tile(short index) {
	return doorlink1_ad[index] & 0x1F;
}

// seg007:0C09
short __pascal far get_doorlink_next(short index) {
	return !(doorlink1_ad[index] & 0x80);
}

// seg007:0C26
short __pascal far get_doorlink_room(short index) {
	return
		((doorlink1_ad[index] & 0x60) >> 5) +
		((doorlink2_ad[index] & 0xE0) >> 3);
}

// seg007:0C53
void __pascal far trigger_button(int playsound,int button_type,int modifier) {
	sbyte link_timer;
	get_curr_tile(curr_tilepos);
	if (button_type == 0) {
		// 0 means currently selected
		button_type = curr_tile;
	}
	if (modifier == -1) {
		// -1 means currently selected
		modifier = curr_modifier;
	}
	link_timer = get_doorlink_timer(modifier);
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
void __pascal far died_on_button() {
	word button_type;
	word modifier;
	button_type = get_curr_tile(curr_tilepos);
	modifier = curr_modifier;
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
void __pascal far animate_button() {
	word var_2;
	if (trob.type >= 0) {
		set_doorlink_timer(curr_modifier, var_2 = get_doorlink_timer(curr_modifier) - 1);
		if (var_2 < 2) {
			trob.type = -1;
			redraw_11h();
		}
	}
}

// seg007:0D72
void __pascal far start_level_door(short room,short tilepos) {
	curr_room_modif[tilepos] = 43; // start fully open
	add_trob(room, tilepos, 3);
}

// seg007:0D93
void __pascal far animate_empty() {
	trob.type = -1;
	redraw_20h();
}

// data:2284
const word y_loose_land[] = {2, 65, 128, 191, 254};
// seg007:0D9D
void __pascal far animate_loose() {
	word room;
	word row;
	word tilepos;
	short anim_type;
	anim_type = trob.type;
	if (anim_type >= 0) {
		++curr_modifier;
		if (curr_modifier & 0x80) {
			// just shaking
			// don't shake on level 13
			if (current_level == 13) return;
			if (curr_modifier >= 0x84) {
				curr_modifier = 0;
				trob.type = -1;
			}
			loose_shake(!curr_modifier);
		} else {
			// something is on the floor
			// should it fall already?
			if (curr_modifier >= 11) {
				curr_modifier = remove_loose(room = trob.room, tilepos = trob.tilepos);
				trob.type = -1;
				curmob.xh = (tilepos % 10) << 2;
				row = tilepos / 10;
				curmob.y = y_loose_land[row + 1];
				curmob.room = room;
				curmob.speed = 0;
				curmob.type = 0;
				curmob.row = row;
				add_mob();
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
void __pascal far loose_shake(int arg_0) {
	word sound_id;
	if (arg_0 || loose_sound[curr_modifier & 0x7F]) {
		do {
			// Sounds 20,21,22: loose floor shaking
			sound_id = prandom(2) + sound_20_loose_shake_1;
		} while(sound_id == last_loose_sound);
		if (sound_flags & sfDigi) {
			last_loose_sound = sound_id;
			// random sample rate (10500..11500)
			//sound_pointers[sound_id]->samplerate = prandom(1000) + 10500;
		}
		play_sound(sound_id);
	}
}

// seg007:0EB8
int __pascal far remove_loose(int room, int tilepos) {
	curr_room_tiles[tilepos] = tiles_0_empty;
	// note: the level type is used to determine the modifier of the empty space left behind
	return tbl_level_type[current_level];
}

// seg007:0ED5
void __pascal far make_loose_fall(byte modifier) {
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
void __pascal far start_chompers() {
	short timing;
	short modifier;
	short tilepos;
	short column;
	timing = 15;
	if ((byte)Char.curr_row < 3) {
		get_room_address(Char.room);
		for (column = 0, tilepos = tbl_line[Char.curr_row];
			column < 10; ++column, ++tilepos
		){
			if (get_curr_tile(tilepos) == tiles_18_chomper) {
				modifier = curr_modifier & 0x7F;
				if (modifier == 0 || modifier >= 6) {
					start_anim_chomper(Char.room, tilepos, timing | (curr_modifier & 0x80));
					timing = next_chomper_timing(timing);
				}
			}
		}
	}
}

// seg007:0F9A
int __pascal far next_chomper_timing(byte timing) {
	// 15,12,9,6,13,10,7,14,11,8,repeat
	timing -= 3;
	if (timing < 6) {
		timing += 10;
	}
	return timing;
}

// seg007:0FB4
void __pascal far loose_make_shake() {
	if (curr_room_modif[curr_tilepos] == 0 && current_level != 13) {
		curr_room_modif[curr_tilepos] = 0x80;
		add_trob(curr_room, curr_tilepos, 1);
	}
}

// seg007:0FE0
void __pascal far do_knock(int room,int tile_row) {
	short tile_col;
	for (tile_col = 0; tile_col < 10; ++tile_col) {
		if (get_tile(room, tile_col, tile_row) == tiles_11_loose) {
			loose_make_shake();
		}
	}
}

// seg007:1010
void __pascal far add_mob() {
	if (mobs_count >= 14) {
		show_dialog("Mobs Overflow");
		return /*0*/; // added
	}
	mobs[mobs_count++] = curmob;
}

// seg007:1041
short __pascal far get_curr_tile(short tilepos) {
	curr_modifier = curr_room_modif[tilepos];
	return curr_tile = curr_room_tiles[tilepos] & 0x1F;
}

// data:43DC
word curmob_index;

// seg007:1063
void __pascal far do_mobs() {
	short n_mobs;
	short index;
	short new_index;
	n_mobs = mobs_count;
	for (curmob_index = 0; n_mobs > curmob_index; ++curmob_index) {
		curmob = mobs[curmob_index];
		move_mob();
		check_loose_fall_on_kid();
		mobs[curmob_index] = curmob;
	}
	new_index = 0;
	for (index = 0; index < mobs_count; ++index) {
		if (mobs[index].speed != -1) {
			mobs[new_index++] = mobs[index];
		}
	}
	mobs_count = new_index;
}

// seg007:110F
void __pascal far move_mob() {
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
void __pascal far move_loose() {
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
void __pascal far loose_land() {
	short button_type;
	short tiletype;
	button_type = 0;
	tiletype = get_tile(curmob.room, curmob.xh >> 2, curmob.row);
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
void __pascal far loose_fall() {
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
void __pascal far redraw_at_cur_mob() {
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
void __pascal far mob_down_a_row() {
	++curmob.row;
	if (curmob.row >= 3) {
		curmob.y -= 192;
		curmob.row = 0;
		curmob.room = level.roomlinks[curmob.room - 1].down;
	}
}

// seg007:13AE
void __pascal far draw_mobs() {
	short index;
	for (index = 0; index < mobs_count; ++index) {
		curmob = mobs[index];
		draw_mob();
	}
}

// seg007:13E5
void __pascal far draw_mob() {
	short tile_row;
	short ypos;
	short top_row;
	short tilepos;
	short tile_col;
	ypos = curmob.y;
	if (curmob.room == drawn_room) {
		if (curmob.y >= 210) return;
	} else if (curmob.room == room_B) {
		if (ABS(ypos) >= 18) return;
		curmob.y += 192;
		ypos = curmob.y;
	} else if (curmob.room == room_A) {
		if (curmob.y < 174) return;
		ypos = curmob.y - 189;
	} else {
		return;
	}
	tile_col = curmob.xh >> 2;
	tile_row = y_to_row_mod4(ypos);
	obj_tilepos = get_tilepos_nominus(tile_col, tile_row);
	++tile_col;
	tilepos = get_tilepos(tile_col, tile_row);
	set_redraw2(tilepos, 1);
	set_redraw_fore(tilepos, 1);
	top_row = y_to_row_mod4(ypos - 18);
	if (top_row != tile_row) {
		tilepos = get_tilepos(tile_col, top_row);
		set_redraw2(tilepos, 1);
		set_redraw_fore(tilepos, 1);
	}
	add_mob_to_objtable(ypos);
}

// seg007:14DE
void __pascal far add_mob_to_objtable(int ypos) {
	word index;
	objtable_type* curr_obj;
	index = objtable_count++;
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
void __pascal far sub_9A8E() {
	// This function is not used.
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &rect_top, &rect_top, 0);
}

// seg007:1556
int __pascal far is_spike_harmful() {
	sbyte modifier;
	modifier = curr_room_modif[curr_tilepos];
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
void __pascal far check_loose_fall_on_kid() {
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
void __pascal far fell_on_your_head() {
	short frame;
	short action;
	frame = Char.frame;
	action = Char.action;
	// loose floors hurt you in frames 5..14 (running) only on level 13
	if (
		(current_level == 13 || (frame < frame_5_start_run || frame >= 15)) &&
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
void __pascal far play_door_sound_if_visible(int sound_id) {
	word has_sound;
	word tilepos;
	word gate_room;
	tilepos = trob.tilepos;
	gate_room = trob.room;
	has_sound = 0;

#ifdef FIX_GATE_SOUNDS
	sbyte has_sound_condition;
	if (fix_gate_sounds)
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
