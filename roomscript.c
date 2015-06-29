#include "common.h"

enum script_op_ids {
    op_70_set_remaining_time = 70,
    op_71_set_next_level = 71,
    op_72_set_next_start_pos = 72,
    op_73_set_next_start_dir_left = 73,
    op_74_set_next_start_dir_right = 74,
    op_75_set_next_cutscene = 75,
    op_76_set_guard_dir_left = 76,
    op_77_set_guard_dir_right = 77,
    op_78_cancel_falling_entry = 78,
    op_79_start_door_is_exit = 79,
};

void reset_room_script(){
    override_next_level = 0;
    override_next_start_pos_doorlink = 0;
    override_next_start_dir_right = 0;
    override_next_start_dir_left = 0;
    override_cutscene = 0;
    override_lvl1_falling_entry = 0;
    is_remaining_time_overridden = 0;
    override_start_door_is_exit = 0;
}


void check_room_script(byte room) {
//    printf("Checking for script tiles in room %d...\n", room);

    // get the address of the tiles and modifiers in this room
    byte* room_tiles = &level.fg[(room-1)*30];
    byte* room_modif = &level.bg[(room-1)*30];

    byte tilepos;
    for(tilepos = 0; tilepos < 30; ++tilepos) {
        byte tile = room_tiles[tilepos] & 0x1F;
        if (tile != tiles_0_empty) continue; // only empty tiles can be script tiles

        byte modifier = room_modif[tilepos]; // modifier is the opcode

        byte adjacent_modifier;
        if (tilepos < 29) adjacent_modifier = room_modif[tilepos+1]; // adjacent tile
        else adjacent_modifier = 0; // the last tile in the room defaults to parameter 0

//        if (modifier >= 70 && modifier <= 78)
//            printf("Detected script tile in room %d, tilepos %d, modifier %d\n", room, tilepos, modifier);

        // Set time remaining (runs only once per game session)
        if (modifier == op_70_set_remaining_time && !is_remaining_time_overridden) {
            rem_min = adjacent_modifier;
            rem_tick = 719;
            is_show_time = 1;
            is_remaining_time_overridden = 1;
        }

        // Override next level (checked right before the next level is loaded)
        if (modifier == op_71_set_next_level) {
            override_next_level = adjacent_modifier;
        }

        // Override start position of next level (checked AFTER next level is loaded)
        if (modifier == op_72_set_next_start_pos) {
            override_next_start_pos_doorlink = adjacent_modifier;
        }

        // Override start direction of next level (checked AFTER next level is loaded)
        if (modifier == op_73_set_next_start_dir_left) {
            override_next_start_dir_left = 1;
            override_next_start_dir_right = 0;
        }
        if (modifier == op_74_set_next_start_dir_right) {
            override_next_start_dir_left = 0;
            override_next_start_dir_right = 1;
        }

        // Set which cutscene will be played before the next level
        if (modifier == op_75_set_next_cutscene) {
            byte par = adjacent_modifier;
            override_cutscene = (adjacent_modifier == 0) ? 255 : adjacent_modifier; // par 0 or 255 cancels the cutscene
        }

        // Set the direction of a guard in a specific room
        if (modifier == op_76_set_guard_dir_left || modifier == op_77_set_guard_dir_right) {
            byte dir = (modifier == op_76_set_guard_dir_left) ? dir_FF_left : dir_0_right;
            int guard_room = adjacent_modifier;
            if (guard_room == 0) {
                if (Guard.direction != dir) Guard.curr_seq = 0x1A8B; // flip direction
                //level.guards_dir[room-1] = dir;
                return;
            }
            level.guards_dir[guard_room-1] = dir;
            //printf("Setting guard direction %d in room %d, current room = %d\n", dir, guard_room, room);
        }

        if (modifier == op_78_cancel_falling_entry) {
            override_lvl1_falling_entry = 1;
        }

        if (modifier == op_79_start_door_is_exit) {
            override_start_door_is_exit = 1;
        }

        ++tilepos; // skip parameter tiles (we could mistake them for script tiles), so don't iterate over them
    }
}

void do_scripted_start_pos_override(byte* room, byte* tilepos) {
    if (override_next_start_pos_doorlink != 0) {
        *room = (byte) get_doorlink_room(override_next_start_pos_doorlink);
        *tilepos = (byte) get_doorlink_tile(override_next_start_pos_doorlink);
        override_next_start_pos_doorlink = 0;
    }
}

void do_scripted_start_dir_override(sbyte* start_dir) {
    if (override_next_start_dir_left) *start_dir = dir_FF_left;
    else if (override_next_start_dir_right) *start_dir = dir_0_right;
    override_next_start_dir_right = 0;
    override_next_start_dir_left = 0;
}

void do_scripted_next_level_override(word* next_level) {
    if (override_next_level != 0) {
        *next_level = override_next_level;
        override_next_level = 0;
    }
}

void do_scripted_cutscene_override(cutscene_ptr_type* cutscene_ptr) {
    if (override_cutscene != 0) {
        switch(override_cutscene) {
            case 255:
                *cutscene_ptr = NULL; // cancel the cutscene
                break;
            case 2:
            case 6:
                *cutscene_ptr = cutscene_2_6;
                break;
            case 4:
                *cutscene_ptr = cutscene_4;
                break;
            case 8:
                *cutscene_ptr = cutscene_8;
                break;
            case 9:
                *cutscene_ptr = cutscene_9;
                break;
            case 12:
                *cutscene_ptr = cutscene_12;
                break;
        }
        override_cutscene = 0;
    }
}
