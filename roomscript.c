#include "common.h"

word override_next_level;
byte override_next_start_pos_doorlink;
byte override_next_start_dir_left;
byte override_next_start_dir_right;
byte override_cutscene;

enum script_op_ids {
    op_70_set_remaining_time = 70,
    op_71_set_next_level = 71,
    op_72_set_next_start_pos = 72,
    op_73_set_next_start_dir_left = 73,
    op_74_set_next_start_dir_right = 74,
    op_75_set_next_cutscene = 75,
    op_76_set_guard_dir_left = 76,
    op_77_set_guard_dir_right = 77,
};

void reset_room_script_overrides(){
    override_next_level = 0;
    override_next_start_pos_doorlink = 0;
    override_next_start_dir_right = 0;
    override_next_start_dir_left = 0;
    override_cutscene = 0;
}


void check_room_script(byte room, byte tilepos) {

    // get the address of the tiles and modifiers in this room
    byte* room_tiles = &level.fg[(room-1)*30];
    byte* room_modif = &level.bg[(room-1)*30];

    byte modifier = room_modif[tilepos];
    byte tile = room_tiles[tilepos] & 0x1F;
    //if (modifier == 77) printf("Detected something in room %d, tilepos %d, tile %d\n", room, tilepos, tile);
    if (tile != tiles_0_empty) return; // ignore all but empty tiles

    if (modifier >= 70) printf("Detected script tile in room %d, tilepos %d, modifier %d\n", room, tilepos, modifier);

    // Set time remaining (runs only once)
    if (modifier == op_70_set_remaining_time) {
        room_modif[tilepos] = 0;
        if (tilepos >= 29) return; // cannot use last tile in the room, leave room for a parameter!
        rem_min = room_modif[tilepos+1];
        rem_tick = 719;
        is_show_time = 1;
        room_modif[tilepos+1] = 0;
    }

    // Override next level (checked right before the next level is loaded)
    if (modifier == op_71_set_next_level) {
        if (tilepos >= 29) return;
        override_next_level = room_modif[tilepos+1];
    }

    // Override start position of next level (checked AFTER next level is loaded)
    if (modifier == op_72_set_next_start_pos) {
        if (tilepos >= 29) return;
        override_next_start_pos_doorlink = room_modif[tilepos+1];
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
        if (tilepos >= 29) return;
        byte par = room_modif[tilepos+1];
        override_cutscene = (par == 0) ? 255 : par; // parameter 0 will cancel the cutscene (equivalent to 255)
    }

    // Set the direction of a guard in a specific room
    if (modifier == op_76_set_guard_dir_left || modifier == op_77_set_guard_dir_right) {
        if (tilepos >= 29) return;
        byte dir = (modifier == op_76_set_guard_dir_left) ? dir_FF_left : dir_0_right;
        int guard_room = room_modif[tilepos+1];
        if (guard_room == 0) {
            Char.direction = dir;
            level.guards_dir[room-1] = dir;
            return;
        }
        level.guards_dir[guard_room-1] = dir;
        //printf("Setting guard direction %d in room %d, current room = %d\n", dir, guard_room, room);
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
    if (override_next_level == 255) {
        override_next_level = 0;
        end_sequence();
        return;
    }
    if (override_next_level != 0) {
        *next_level = override_next_level;
        override_next_level = 0;
    }
    // use parameter 255 to skip to the ending sequence
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
