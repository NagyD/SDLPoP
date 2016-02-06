#define NULL 0

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short word;
typedef unsigned long dword;

typedef struct rect_type {
    short top,left,bottom,right;
} rect_type;

enum tiles {
    TILE_EMPTY = 0,
    TILE_FLOOR = 1,
    TILE_SPIKES = 2,
    TILE_PILLAR = 3,
    TILE_GATE = 4,
    TILE_STUCK = 5,
    TILE_CLOSER = 6, // a.k.a. drop button
    TILE_DOORTOP_WITH_FLOOR = 7, // a.k.a. tapestry
    TILE_BIGPILLAR_BOTTOM = 8,
    TILE_BIGPILLAR_TOP = 9,
    TILE_POTION = 10,
    TILE_LOOSE = 11,
    TILE_DOORTOP = 12, // a.k.a. tapestry top
    TILE_MIRROR = 13,
    TILE_DEBRIS = 14, // a.k.a. broken floor
    TILE_OPENER = 15, // a.k.a. raise button
    TILE_LEVEL_DOOR_LEFT = 16, // a.k.a. exit door
    TILE_LEVEL_DOOR_RIGHT = 17,
    TILE_CHOMPER = 18,
    TILE_TORCH = 19,
    TILE_WALL = 20,
    TILE_SKELETON = 21,
    TILE_SWORD = 22,
    TILE_BALCONY_LEFT = 23,
    TILE_BALCONY_RIGHT = 24,
    TILE_LATTICE_PILLAR = 25,
    TILE_LATTICE_DOWN = 26, // a.k.a. lattice support
    TILE_LATTICE_SMALL = 27,
    TILE_LATTICE_LEFT = 28,
    TILE_LATTICE_RIGHT = 29,
    TILE_TORCH_WITH_DEBRIS = 30
};

#pragma pack(push,1)
typedef struct link_type {
    byte left,right,up,down;
} link_type;

typedef struct level_type {
    byte fg[720];
    byte bg[720];
    byte doorlinks1[256];
    byte doorlinks2[256];
    link_type roomlinks[24];
    byte used_rooms;
    byte roomxs[24];
    byte roomys[24];
    byte fill_1[15];
    byte start_room;
    byte start_pos;
    sbyte start_dir;
    byte fill_2[4];
    byte guards_tile[24];
    byte guards_dir[24];
    byte guards_x[24];
    byte guards_seq_lo[24];
    byte guards_skill[24];
    byte guards_seq_hi[24];
    byte guards_color[24];
    byte fill_3[18];
} level_type;
#pragma pack(pop)

typedef struct char_type {
    byte frame;
    byte x;
    byte y;
    sbyte direction;
    sbyte curr_col;
    sbyte curr_row;
    byte action;
    sbyte fall_x;
    sbyte fall_y;
    byte room;
    byte repeat;
    byte charid;
    byte sword;
    sbyte alive;
    word curr_seq;
} char_type;

enum charids {
    CHAR_KID      = 0,
    CHAR_SHADOW   = 1,
    CHAR_GUARD    = 2,
    //charid_3          = 3,
    CHAR_SKELETON = 4,
    CHAR_PRINCESS = 5,
    CHAR_VIZIER   = 6,
    CHAR_MOUSE   = 0x18
};

enum directions {
    DIR_RIGHT = 0x00,
    DIR_NONE = 0x56,
    DIR_LEFT = -1
};

enum actions {
    ACTION_STAND         = 0,
    ACTION_RUN_JUMP      = 1,
    ACTION_HANG_CLIMB    = 2,
    ACTION_IN_MIDAIR     = 3,
    ACTION_IN_FREEFALL   = 4,
    ACTION_BUMPED        = 5,
    ACTION_HANG_STRAIGHT = 6,
    ACTION_TURN          = 7,
    ACTION_HURT         = 99,
};

enum soundids {
    SOUND_FELL_TO_DEATH             = 0,
    SOUND_FALLING                   = 1,
    SOUND_TILE_CRASHING             = 2,
    SOUND_BUTTON_PRESSED            = 3,
    SOUND_GATE_CLOSING              = 4,
    SOUND_GATE_OPENING              = 5,
    SOUND_GATE_CLOSING_FAST         = 6,
    SOUND_GATE_STOP                 = 7,
    SOUND_BUMPED                    = 8,
    SOUND_GRAB                      = 9,
    SOUND_SWORD_VS_SWORD            = 10,
    SOUND_SWORD_MOVING              = 11,
    SOUND_GUARD_HURT                = 12,
    SOUND_KID_HURT                  = 13,
    SOUND_LEVELDOOR_CLOSING         = 14,
    SOUND_LEVELDOOR_SLIDING         = 15,
    SOUND_MEDIUM_LAND               = 16,
    SOUND_SOFT_LAND                 = 17,
    SOUND_DRINK                     = 18,
    SOUND_DRAW_SWORD                = 19,
    SOUND_LOOSE_SHAKE_1             = 20,
    SOUND_LOOSE_SHAKE_2             = 21,
    SOUND_LOOSE_SHAKE_3             = 22,
    SOUND_FOOTSTEP                  = 23,
    SOUND_DEATH_REGULAR             = 24,
    SOUND_PRESENTATION              = 25,
    SOUND_EMBRACE                   = 26,
    SOUND_CUTSCENE_2_4_6_12         = 27,
    SOUND_DEATH_IN_FIGHT            = 28,
    SOUND_MEET_JAFFAR               = 29,
    SOUND_BIG_POTION                = 30,
    //sound_31 = 31,
    SOUND_SHADOW_MUSIC              = 32,
    SOUND_SMALL_POTION              = 33,
    //sound_34 = 34,
    SOUND_CUTSCENE_8_9              = 35,
    SOUND_OUT_OF_TIME               = 36,
    SOUND_VICTORY                   = 37,
    SOUND_BLINK                     = 38,
    SOUND_LOW_WEIGHT                = 39,
    SOUND_CUTSCENE_12_SHORT_TIME    = 40,
    SOUND_END_LEVEL_MUSIC           = 41,
    //sound_42 = 42,
    SOUND_VICTORY_JAFFAR            = 43,
    SOUND_SKEL_ALIVE                = 44,
    SOUND_JUMP_THROUGH_MIRROR       = 45,
    SOUND_CHOMPED                   = 46,
    SOUND_CHOMPER                   = 47,
    SOUND_SPIKED                    = 48,
    SOUND_SPIKES                    = 49,
    SOUND_STORY_2_PRINCESS          = 50,
    SOUND_PRINCESS_DOOR_OPENING     = 51,
    SOUND_STORY_4_JAFFAR_LEAVES     = 52,
    SOUND_STORY_3_JAFFAR_COMES      = 53,
    SOUND_INTRO_MUSIC               = 54,
    SOUND_STORY_1_ABSENCE           = 55,
    SOUND_ENDING_MUSIC              = 56,
};

enum seqids {
    SEQ_TURN = 5,
    SEQ_FALL = 7,
    SEQ_RUN = 84,
};

enum colorids {
    COLOR_BLACK = 0,
    COLOR_DARKBLUE = 1,
    COLOR_DARKGREEN = 2,
    COLOR_TURQUOISE = 3,
    COLOR_RED = 4,
    COLOR_PURPLE = 5,
    COLOR_GOLD = 6,
    COLOR_GREY = 7,
    COLOR_DARKGREY = 8,
    COLOR_BLUE = 9,
    COLOR_GREEN = 10,
    COLOR_LIGHTBLUE = 11,
    COLOR_RED = 12,
    COLOR_VIOLET = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15,
};

enum rowids {
    ROW_ABOVE  = -1,
    ROW_TOP    = 0,
    ROW_MIDDLE = 1,
    ROW_BOTTOM = 2,
    ROW_BELOW  = 3,
};

enum potsizeids {
    POT_SMALL = 0,
    POT_BIG = 1,
};

// SCRIPT FUNCTION PROTOTYPES

// Registered variables will be saved in and loaded from savestates (replays, quicksave)
void register_savestate_variable_explicitly(void* source, int num_bytes, char* variable_name);
// You can use this macro instead of the explicit function (so you don't need to think too much about addresses, sizes and variable names)
#define register_savestate_variable(var) register_savestate_variable_explicitly(&var, sizeof(var), #var)

// accessible game functions
void play_sound(int sound_id);
void stop_sounds(void);
void draw_kid_hp(short curr_hp, short max_hp);
void take_hp(int count);
void set_hp_full(void);
void set_char_sequence(short seq_index);

// "encapsulated" data accessors:
word get_minutes_remaining(void);
word get_ticks_remaining(void);
void set_time_remaining(word minutes, word ticks);

void select_tile_at_col_row(int room, int column, int row);
void select_tile_at_tilepos(word room, word tilepos);
// select_tile() is "overloaded"; it can be used either with:
//   2 arguments --> calls: select_tile_at_tilepos()
//   3 arguments --> calls: select_tile_at_col_row()
#define GET_OVERLOADED_FUNC(_1, _2, _3, NAME, ...) NAME
#define select_tile(...) GET_OVERLOADED_FUNC(__VA_ARGS__, select_tile_at_col_row,select_tile_at_tilepos )(__VA_ARGS__)

byte get_curr_tile(void);
byte get_curr_modifier(void);

void set_curr_tile(byte new_tile);
void set_curr_modifier(byte new_modifier);
void set_curr_tile_and_modifier(byte new_tile, byte new_modifier);

void set_tile(word room, word tilepos, byte new_tile);
void set_modifier(word room, word tilepos, byte new_modifier);
void set_tile_and_modifier(word room, word tilepos, byte new_tile, byte new_modifier);

word get_hp(void);
void set_hp(word new_hp);
word get_max_hp(void);
void set_max_hp(word new_max_hp);

void set_flash(word color, word duration);

void set_potion_color(word color);        // use only in custom_potion_anim
void set_potion_pot_size(word pot_size);  // use only in custom_potion_anim

void show_time(void);

short have_sword(void);
void set_have_sword(short kid_has_sword);

word get_curr_level(void);

void override_level_start_sequence(word sequence_index);
void disable_level1_music(void);


// DATA

extern level_type* ptr_level;
#define level (*ptr_level)

extern char_type* ptr_kid;
#define kid (*ptr_kid)

extern char_type* ptr_guard;
#define guard (*ptr_guard)
