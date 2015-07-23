#include "common.h"

#define SEQTBL_BASE 0x196E
#define SEQTBL_0 (seqtbl - SEQTBL_BASE)

// This expands a two-byte number into two comma-separated bytes, used for the JMP destinations
#define DW(data_word) data_word & 0x00FF, ((data_word & 0xFF00) >> 8)

// Insertion and deletion into the seqtable is not practical at the moment...
// @Todo: add a way to appropriately shift seqtbl offsets when inserting/deleting
//#define SEQTBL_OFFSET(index) index + SEQTBL_BASE
//#define running SEQTBL_OFFSET(0)
//#define startrun SEQTBL_OFFSET(5)
//#define stand SEQTBL_OFFSET(50)
//#define goalertstand SEQTBL_OFFSET(56)
//#define arise SEQTBL_OFFSET(62)
//...

// data:196E
const byte seqtbl[] = {

        // running
        SEQ_ACTION, actions_1_run_jump, SEQ_JMP, DW(0x1981), // goto running: frame 7

        // startrun
        SEQ_ACTION, actions_1_run_jump, frame_1_start_run,
        frame_2_start_run, frame_3_start_run, frame_4_start_run,
        SEQ_DX, 8, frame_5_start_run,
        SEQ_DX, 3, frame_6_start_run,
        SEQ_DX, 3, /* "runcyc1" */ frame_7_run,
        SEQ_DX, 5, frame_8_run,
        SEQ_DX, 1, SEQ_SOUND, SND_FOOTSTEP, frame_9_run,
        SEQ_DX, 2, frame_10_run,
        SEQ_DX, 4, frame_11_run,
        SEQ_DX, 5, frame_12_run,
        SEQ_DX, 2, SEQ_SOUND, SND_FOOTSTEP, frame_13_run,
        SEQ_DX, 3, frame_14_run,
        SEQ_DX, 4, SEQ_JMP, DW(0x1981), // goto "runcyc1"

        // stand
        SEQ_ACTION, actions_0_stand, frame_15_stand,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // alert stand
        SEQ_ACTION, actions_1_run_jump, /* ":loop" */ frame_166_stand_inactive,
        SEQ_JMP, DW(0x19A8), // goto ":loop"

        // arise (skeleton)
        SEQ_ACTION, actions_5_bumped, SEQ_DX, 10, frame_177_spiked,
        frame_177_spiked,
        SEQ_DX, (byte)-7, SEQ_DY, (byte)-2, frame_178_chomped,
        SEQ_DX, 5, SEQ_DY, 2, frame_166_stand_inactive,
        SEQ_DX, (byte)-1, SEQ_JMP, DW(0x19D2), // goto "ready"

        // guard engarde
        SEQ_JMP, DW(0x19D2), // goto "ready"

        // engarde
        SEQ_ACTION, actions_1_run_jump,
        SEQ_DX, 2, frame_207_draw_1,
        frame_208_draw_2,
        SEQ_DX, 2, frame_209_draw_3,
        SEQ_DX, 2, frame_210_draw_4,
        SEQ_DX, 3, /* "ready" */ SEQ_ACTION, actions_1_run_jump, SEQ_SOUND, SND_SILENT, frame_158_stand_with_sword,
        frame_170_stand_with_sword,
        /*:loop*/ frame_171_stand_with_sword,
        SEQ_JMP, DW(0x19D8), // goto ":loop"

        // stabbed
        SEQ_ACTION, actions_5_bumped, SEQ_SET_FALL, (byte)-1, 0, frame_172_jumpfall_2,
        SEQ_DX, (byte)-1, SEQ_DY, 1, frame_173_jumpfall_3,
        SEQ_DX, (byte)-1, frame_174_jumpfall_4,
        SEQ_DX, (byte)-1, SEQ_DY, 2, // frame 175 is commented out in the Apple II source
        SEQ_DX, (byte)-2, SEQ_DY, 1,
        SEQ_DX, (byte)-5, SEQ_DY, (byte)-4,
        SEQ_JMP, DW(0x1A4D), // goto "guy8"

        // strike - advance
        SEQ_ACTION, actions_1_run_jump, SEQ_SET_FALL, 1, 0, frame_155_guy_7,
        SEQ_DX, 2, frame_165_walk_with_sword,
        SEQ_DX, (byte)-2, SEQ_JMP, DW(0x19D2), // goto "ready"

        // strike - retreat
        SEQ_ACTION, actions_1_run_jump, SEQ_SET_FALL, (byte)-1, 0, frame_155_guy_7,
        frame_156_guy_8,
        frame_157_walk_with_sword,
        frame_158_stand_with_sword,
        SEQ_JMP, DW(0x1A2E), // goto "retreat"

        // advance
        SEQ_ACTION, actions_1_run_jump, SEQ_SET_FALL, 1, 0,
        SEQ_DX, 2, frame_163_fighting,
        SEQ_DX, 4, frame_164_fighting,
        frame_165_walk_with_sword,
        SEQ_JMP, DW(0x19D2), // goto "ready"

        // fast advance
        SEQ_ACTION, actions_1_run_jump, SEQ_SET_FALL, 1, 0, SEQ_DX, 6, frame_164_fighting,
        frame_165_walk_with_sword,
        SEQ_JMP, DW(0x19D2), // goto "ready"

        // retreat
        SEQ_ACTION, actions_1_run_jump, SEQ_SET_FALL, (byte)-1, 0, SEQ_DX, (byte)-3, frame_160_fighting,
        SEQ_DX, (byte)-2, frame_157_walk_with_sword,
        SEQ_JMP, DW(0x19D2), // goto "ready"

        // strike
        SEQ_ACTION, actions_1_run_jump, SEQ_SET_FALL, (byte)-1, 0, frame_168_back,
        SEQ_ACTION, actions_1_run_jump, frame_151_strike_1,
        /* "guy4" */ SEQ_ACTION, actions_1_run_jump, frame_152_strike_2,
        frame_153_strike_3,
        frame_154_poking,
        /* "guy7" */ SEQ_ACTION, actions_5_bumped, frame_155_guy_7,
        /* "guy8" */ SEQ_ACTION, actions_1_run_jump, frame_156_guy_8,
        frame_157_walk_with_sword,
        SEQ_JMP, DW(0x19D2), // goto "ready"

        // blocked strike
        SEQ_ACTION, actions_1_run_jump, frame_167_blocked,
        SEQ_JMP, DW(0x1A4A), // goto "guy7"

        // block to strike
        // "blocktostrike"
        frame_162_block_to_strike,
        SEQ_JMP, DW(0x1A45), // goto "guy4"

        // ready block
        frame_169_begin_block,
        /* "blocking" */ frame_150_parry,
        SEQ_JMP, DW(0x19D2), // goto "ready"

        // strike to block
        frame_159_fighting,
        frame_160_fighting,
        SEQ_JMP, DW(0x1A5F), // goto "blocking"

        // land en garde
        SEQ_ACTION, actions_1_run_jump, SEQ_KNOCK_DOWN, SEQ_JMP, DW(0x19D2), // goto "ready"

        // bump en garde (forward)
        SEQ_ACTION, actions_5_bumped, SEQ_DX, (byte)-8, SEQ_JMP, DW(0x19D2), // goto "ready"

        // bump en garde (back)
        SEQ_ACTION, actions_5_bumped, frame_160_fighting,
        frame_157_walk_with_sword,
        SEQ_JMP, DW(0x19D2), // goto "ready"

        // flee
        SEQ_ACTION, actions_7_turn, SEQ_DX, (byte)-8, SEQ_JMP, DW(0x1B39), // goto "turn"

        // turn en garde
        SEQ_ACTION, actions_5_bumped, SEQ_FLIP, SEQ_DX, 5, SEQ_JMP, DW(0x1A2E), // goto "retreat"

        // alert turn (for enemies)
        SEQ_ACTION, actions_5_bumped, SEQ_FLIP, SEQ_DX, 18, SEQ_JMP, DW(0x19A6), // goto "goalertstand"

        // standing jump
        SEQ_ACTION, actions_1_run_jump, frame_16_standing_jump_1,
        frame_17_standing_jump_2,
        SEQ_DX, 2, frame_18_standing_jump_3,
        SEQ_DX, 2, frame_19_standing_jump_4,
        SEQ_DX, 2, frame_20_standing_jump_5,
        SEQ_DX, 2, frame_21_standing_jump_6,
        SEQ_DX, 2, frame_22_standing_jump_7,
        SEQ_DX, 7, frame_23_standing_jump_8,
        SEQ_DX, 9, frame_24_standing_jump_9,
        SEQ_DX, 5, SEQ_DY, (byte)-6, /* "sjland" */ frame_25_standing_jump_10,
        SEQ_DX, 1, SEQ_DY, 6, frame_26_standing_jump_11,
        SEQ_DX, 4, SEQ_KNOCK_DOWN, SEQ_SOUND, SND_FOOTSTEP, frame_27_standing_jump_12,
        SEQ_DX, (byte)-3, frame_28_standing_jump_13,
        SEQ_DX, 5, frame_29_standing_jump_14,
        SEQ_SOUND, SND_FOOTSTEP, frame_30_standing_jump_15,
        frame_31_standing_jump_16,
        frame_32_standing_jump_17,
        frame_33_standing_jump_18,
        SEQ_DX, 1, SEQ_JMP, DW(0x19A0), // goto "stand"

        // running jump
        SEQ_ACTION, actions_1_run_jump, SEQ_SOUND, SND_FOOTSTEP, frame_34_start_run_jump_1,
        SEQ_DX, 5, frame_35_start_run_jump_2,
        SEQ_DX, 6, frame_36_start_run_jump_3,
        SEQ_DX, 3, frame_37_start_run_jump_4,
        SEQ_DX, 5, SEQ_SOUND, SND_FOOTSTEP, frame_38_start_run_jump_5,
        SEQ_DX, 7, frame_39_start_run_jump_6,
        SEQ_DX, 12, SEQ_DY, (byte)-3, frame_40_running_jump_1,
        SEQ_DX, 8, SEQ_DY, (byte)-9, frame_41_running_jump_2,
        SEQ_DX, 8, SEQ_DY, (byte)-2, frame_42_running_jump_3,
        SEQ_DX, 4, SEQ_DY, 11, frame_43_running_jump_4,
        SEQ_DX, 4, SEQ_DY, 3, /* "rjlandrun" */ frame_44_running_jump_5,
        SEQ_DX, 5, SEQ_KNOCK_DOWN, SEQ_SOUND, SND_FOOTSTEP, SEQ_JMP, DW(0x1981), // goto "runcyc1"

        // run dive roll
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, 1, frame_107_fall_land_1,
        SEQ_DX, 2, SEQ_DX, 2, frame_108_fall_land_2,
        SEQ_DX, 2, frame_109_crouch,
        SEQ_DX, 2, frame_109_crouch,
        SEQ_DX, 2, /* ":crouch" */frame_109_crouch,
        SEQ_JMP, DW(0x1B16), // goto ":crouch"
        0x00, // stand dive roll; not implemented

        // crawl
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, 1, frame_110_stand_up_from_crouch_1,
        frame_111_stand_up_from_crouch_2,
        SEQ_DX, 2, frame_112_stand_up_from_crouch_3,
        SEQ_DX, 2, frame_108_fall_land_2,
        SEQ_DX, 2, /* ":crouch" */ frame_109_crouch,
        SEQ_JMP, DW(0x1B29), // goto ":crouch"

        // turn draw
        SEQ_ACTION, actions_7_turn, SEQ_FLIP, SEQ_DX, 6, frame_45_turn,
        SEQ_DX, 1, frame_46_turn,
        SEQ_JMP, DW(0x19C4), // goto "engarde"

        // turn
        SEQ_ACTION, actions_7_turn, SEQ_FLIP, SEQ_DX, 6, frame_45_turn,
        SEQ_DX, 1, frame_46_turn,
        SEQ_DX, 2, frame_47_turn,
        SEQ_DX, (byte)-1, /* "finishturn" */ frame_48_turn,
        SEQ_DX, 0x01, frame_49_turn,
        SEQ_DX, 0xFE, frame_50_turn,
        frame_51_turn,
        frame_52_turn,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // turnrun (from frmame 48)
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, (byte)-1, SEQ_JMP, DW(0x1975), // goto "runstt1"

        // runturn
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, 1, frame_53_runturn,
        SEQ_DX, 1, SEQ_SOUND, SND_FOOTSTEP, frame_54_runturn,
        SEQ_DX, 8 ,frame_55_runturn,
        SEQ_SOUND, SND_FOOTSTEP, frame_56_runturn,
        SEQ_DX, 7, frame_57_runturn,
        SEQ_DX, 3, frame_58_runturn,
        SEQ_DX, 1, frame_59_runturn,
        frame_60_runturn,
        SEQ_DX, 2, frame_61_runturn,
        SEQ_DX, (byte)-1, frame_62_runturn,
        frame_63_runturn,
        frame_64_runturn,
        SEQ_DX, (byte)-1, frame_65_runturn,
        SEQ_DX, (byte)-14, SEQ_FLIP, SEQ_JMP, DW(0x1995), // goto "runcyc7"

        // fightfall (backward)
        SEQ_ACTION, actions_3_in_midair, SEQ_DY, (byte)-1, frame_102_start_fall_1,
        SEQ_DX, (byte)-2, SEQ_DY, 6, frame_103_start_fall_2,
        SEQ_DX, (byte)-2, SEQ_DY, 9, frame_104_start_fall_3,
        SEQ_DX, (byte)-1, SEQ_DY, 12, frame_105_start_fall_4,
        SEQ_DX, (byte)-3, SEQ_SET_FALL, 0, 15, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // enemy fight fall
        SEQ_ACTION, actions_3_in_midair, SEQ_DY, (byte)-1, SEQ_DX, (byte)-2, frame_102_start_fall_1,
        SEQ_DX, (byte)-3, SEQ_DY, 6, frame_103_start_fall_2,
        SEQ_DX, (byte)-3, SEQ_DY, 9, frame_104_start_fall_3,
        SEQ_DX, (byte)-2, SEQ_DY, 12, frame_105_start_fall_4,
        SEQ_DX, (byte)-3, SEQ_SET_FALL, 0, 15, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // enemy fight fall forward
        SEQ_ACTION, actions_3_in_midair, SEQ_DX, 1, SEQ_DY, (byte)-1, frame_102_start_fall_1,
        SEQ_DX, 2, SEQ_DY, 6, frame_103_start_fall_2,
        SEQ_DX, (byte)-1, SEQ_DY, 9, frame_104_start_fall_3,
        SEQ_DY, 12, frame_105_start_fall_4,
        SEQ_DX, (byte)-2, SEQ_SET_FALL, 1, 15, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // stepfall
        SEQ_ACTION, actions_3_in_midair, SEQ_DX, 1, SEQ_DY, 3, SEQ_JMP_IF_FEATHER, DW(0x1C06), // goto "stepfloat"
        /* "fall1" */ frame_102_start_fall_1,
        SEQ_DX, 2, SEQ_DY, 6, frame_103_start_fall_2,
        SEQ_DX, (byte)-1, SEQ_DY, 9, frame_104_start_fall_3,
        SEQ_DY, 12, frame_105_start_fall_4,
        SEQ_DX, (byte)-2, SEQ_SET_FALL, 1, 15, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // patchfall
        SEQ_DX, (byte)-1, SEQ_DY, (byte)-3, SEQ_JMP, DW(0x1BE4), // goto "fall1"

        // stepfall2 (from frame 12)
        SEQ_DX, 1, SEQ_JMP, DW(0x1BDB), // goto "stepfall"

        // stepfloat
        frame_102_start_fall_1,
        SEQ_DX, 2, SEQ_DY, 3, frame_103_start_fall_2,
        SEQ_DX, (byte)-1, SEQ_DY, 4, frame_104_start_fall_3,
        SEQ_DY, 5, frame_105_start_fall_4,
        SEQ_DX, (byte)-2, SEQ_SET_FALL, 1, 6, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // jump fall (from standing jump)
        SEQ_ACTION, actions_3_in_midair, SEQ_DX, 1, SEQ_DY, 3, frame_102_start_fall_1,
        SEQ_DX, 2, SEQ_DY, 6, frame_103_start_fall_2,
        SEQ_DX, 1, SEQ_DY, 9, frame_104_start_fall_3,
        SEQ_DX, 2, SEQ_DY, 12, frame_105_start_fall_4,
        SEQ_SET_FALL, 2, 15, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // running jump fall
        SEQ_ACTION, actions_3_in_midair, SEQ_DX, 1, SEQ_DY, 3, frame_102_start_fall_1,
        SEQ_DX, 3, SEQ_DY, 6, frame_103_start_fall_2,
        SEQ_DX, 2, SEQ_DY, 9, frame_104_start_fall_3,
        SEQ_DX, 3, SEQ_DY, 12, frame_105_start_fall_4,
        SEQ_SET_FALL, 3, 15, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // jumphang (medium: DX = 0)
        SEQ_ACTION, actions_1_run_jump, frame_67_start_jump_up_1,
        frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
        frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang, frame_76_jumphang,
        frame_77_jumphang,
        SEQ_ACTION, actions_2_hang_climb, frame_78_jumphang, frame_79_jumphang, frame_80_jumphang,
        SEQ_JMP, DW(0x1CA1), // goto "hang"

        // jumphang (long: DX = 4)
        SEQ_ACTION, actions_1_run_jump, frame_67_start_jump_up_1,
        frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
        frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang, frame_76_jumphang,
        frame_77_jumphang,
        SEQ_ACTION, actions_2_hang_climb, SEQ_DX, 1, frame_78_jumphang,
        SEQ_DX, 2, frame_79_jumphang,
        SEQ_DX, 1, frame_80_jumphang,
        SEQ_JMP, DW(0x1CA1), // goto "hang"

        // jumpbackhang
        SEQ_ACTION, actions_1_run_jump, frame_67_start_jump_up_1,
        frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
        frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang, frame_76_jumphang,
        SEQ_DX, (byte)-1, frame_77_jumphang,
        SEQ_ACTION, actions_2_hang_climb, SEQ_DX, (byte)-2, frame_78_jumphang,
        SEQ_DX, (byte)-1, frame_79_jumphang,
        SEQ_DX, (byte)-1, frame_80_jumphang,
        SEQ_JMP, DW(0x1CA1), // goto "hang"

        // hang
        SEQ_ACTION, actions_2_hang_climb, frame_91_hanging_5,
        /* "hang1" */ frame_90_hanging_4, frame_89_hanging_3, frame_88_hanging_2,
        frame_87_hanging_1, frame_87_hanging_1, frame_87_hanging_1, frame_88_hanging_2,
        frame_89_hanging_3, frame_90_hanging_4, frame_91_hanging_5, frame_92_hanging_6,
        frame_93_hanging_7, frame_94_hanging_8, frame_95_hanging_9, frame_96_hanging_10,
        frame_97_hanging_11, frame_98_hanging_12, frame_99_hanging_13, frame_97_hanging_11,
        frame_96_hanging_10, frame_95_hanging_9, frame_94_hanging_8, frame_93_hanging_7,
        frame_92_hanging_6, frame_91_hanging_5, frame_90_hanging_4, frame_89_hanging_3,
        frame_88_hanging_2, frame_87_hanging_1, frame_88_hanging_2, frame_89_hanging_3,
        frame_90_hanging_4, frame_91_hanging_5, frame_92_hanging_6, frame_93_hanging_7,
        frame_94_hanging_8, frame_95_hanging_9, frame_96_hanging_10, frame_95_hanging_9,
        frame_94_hanging_8, frame_93_hanging_7, frame_92_hanging_6,
        SEQ_JMP, DW(0x1D25), // goto "hangdrop"

        // hangstraight
        SEQ_ACTION, actions_6_hang_straight, frame_92_hanging_6, // Apple II source has a bump sound here
        frame_93_hanging_7, frame_93_hanging_7, frame_92_hanging_6, frame_92_hanging_6,
        /* ":loop" */frame_91_hanging_5,
        SEQ_JMP, DW(0x1CD8), // goto ":loop"

        // climbfail
        frame_135_climbing_1, frame_136_climbing_2, frame_137_climbing_3, frame_137_climbing_3,
        frame_138_climbing_4, frame_138_climbing_4, frame_138_climbing_4, frame_138_climbing_4,
        frame_137_climbing_3, frame_136_climbing_2, frame_135_climbing_1,
        SEQ_DX, (byte)-7, SEQ_JMP, DW(0x1D25), // goto "hangdrop"

        // climbdown
        SEQ_ACTION, actions_1_run_jump, frame_148_climbing_14,
        frame_145_climbing_11, frame_144_climbing_10, frame_143_climbing_9, frame_142_climbing_8,
        frame_141_climbing_7,
        SEQ_DX, (byte)-5, SEQ_DY, 63, SEQ_DOWN, SEQ_ACTION, actions_3_in_midair, frame_140_climbing_6,
        frame_138_climbing_4, frame_136_climbing_2,
        frame_91_hanging_5,
        SEQ_ACTION, actions_2_hang_climb, SEQ_JMP, DW(0x1CA4), // goto "hang1"

        // climbup
        SEQ_ACTION, actions_1_run_jump, frame_135_climbing_1,
        frame_136_climbing_2, frame_137_climbing_3, frame_138_climbing_4,
        frame_139_climbing_5, frame_140_climbing_6,
        SEQ_DX, 5, SEQ_DY, (byte)-63, SEQ_UP, frame_141_climbing_7,
        frame_142_climbing_8, frame_143_climbing_9, frame_144_climbing_10, frame_145_climbing_11,
        frame_146_climbing_12, frame_147_climbing_13, frame_148_climbing_14,
        SEQ_ACTION, actions_5_bumped, // to clear flags
        frame_149_climbing_15,
        SEQ_ACTION, actions_1_run_jump, frame_118_stand_up_from_crouch_9, frame_119_stand_up_from_crouch_10,
        SEQ_DX, 1, SEQ_JMP, DW(0x19A0), // goto "stand"

        // hangdrop
        frame_81_hangdrop_1, frame_82_hangdrop_2,
        SEQ_ACTION, actions_5_bumped, frame_83_hangdrop_3,
        SEQ_ACTION, actions_1_run_jump, SEQ_KNOCK_DOWN, SEQ_SOUND, SND_SILENT,
        frame_84_hangdrop_4, frame_85_hangdrop_5,
        SEQ_DX, actions_3_in_midair, SEQ_JMP, DW(0x19A0), // goto "stand"

        // hangfall
        SEQ_ACTION, actions_3_in_midair, frame_81_hangdrop_1,
        SEQ_DY, 6, frame_81_hangdrop_1,
        SEQ_DY, 9, frame_81_hangdrop_1,
        SEQ_DY, 12, SEQ_DX, 2, SEQ_SET_FALL, 0, 12, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // freefall
        SEQ_ACTION, actions_4_in_freefall, /* ":loop" */ frame_106_fall,
        SEQ_JMP, DW(0x1D4B), // goto :loop

        // runstop
        SEQ_ACTION, actions_1_run_jump, frame_53_runturn,
        SEQ_DX, 2, SEQ_SOUND, SND_FOOTSTEP, frame_54_runturn,
        SEQ_DX, 7, frame_55_runturn,
        SEQ_SOUND, SND_FOOTSTEP, frame_56_runturn,
        SEQ_DX, 2, frame_49_turn,
        SEQ_DX, (byte)-2, frame_50_turn,
        frame_51_turn, frame_52_turn,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // jump up (and touch ceiling)
        SEQ_ACTION, actions_1_run_jump, frame_67_start_jump_up_1,
        frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
        frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang,
        frame_76_jumphang, frame_77_jumphang, frame_78_jumphang,
        SEQ_ACTION, actions_0_stand, SEQ_KNOCK_UP, frame_79_jumphang,
        SEQ_JMP, DW(0x1D25), // goto "hangdrop"

        // highjump (no ceiling above)
        SEQ_ACTION, actions_1_run_jump, frame_67_start_jump_up_1,
        frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
        frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang,
        frame_76_jumphang, frame_77_jumphang, frame_78_jumphang, frame_79_jumphang,
        SEQ_DY, (byte)-4, frame_79_jumphang,
        SEQ_DY, (byte)-2, frame_79_jumphang,
        frame_79_jumphang,
        SEQ_DY, 2, frame_79_jumphang,
        SEQ_DY, 4, SEQ_JMP, DW(0x1D25), // goto "hangdrop"

        // superhijump (when weightless)
        frame_67_start_jump_up_1, frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang,
        frame_71_jumphang, frame_72_jumphang, frame_73_jumphang, frame_74_jumphang,
        frame_75_jumphang, frame_76_jumphang,
        SEQ_DY, (byte)-1, frame_77_jumphang,
        SEQ_DY, (byte)-3, frame_78_jumphang,
        SEQ_DY, (byte)-4, frame_79_jumphang,
        SEQ_DY, (byte)-10, frame_79_jumphang,
        SEQ_DY, (byte)-9, frame_79_jumphang,
        SEQ_DY, (byte)-8, frame_79_jumphang,
        SEQ_DY, (byte)-7, frame_79_jumphang,
        SEQ_DY, (byte)-6, frame_79_jumphang,
        SEQ_DY, (byte)-5, frame_79_jumphang,
        SEQ_DY, (byte)-4, frame_79_jumphang,
        SEQ_DY, (byte)-3, frame_79_jumphang,
        SEQ_DY, (byte)-2, frame_79_jumphang,
        SEQ_DY, (byte)-2, frame_79_jumphang,
        SEQ_DY, (byte)-1, frame_79_jumphang,
        SEQ_DY, (byte)-1, frame_79_jumphang,
        SEQ_DY, (byte)-1, frame_79_jumphang,
        frame_79_jumphang, frame_79_jumphang, frame_79_jumphang,
        SEQ_DY, 1, frame_79_jumphang,
        SEQ_DY, 1, frame_79_jumphang,
        SEQ_DY, 2, frame_79_jumphang,
        SEQ_DY, 2, frame_79_jumphang,
        SEQ_DY, 3, frame_79_jumphang,
        SEQ_DY, 4, frame_79_jumphang,
        SEQ_DY, 5, frame_79_jumphang,
        SEQ_DY, 6, frame_79_jumphang,
        SEQ_SET_FALL, 0, 6, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // fall hang
        SEQ_ACTION, actions_3_in_midair, frame_80_jumphang,
        SEQ_JMP, DW(0x1CA1), // goto "hang"

        // bump
        SEQ_ACTION, actions_5_bumped, SEQ_DX, (byte)-4, frame_50_turn,
        frame_51_turn, frame_52_turn,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        #ifdef FIX_WALL_BUMP_TRIGGERS_TILE_BELOW
        #define BUMPFALL_ACTION actions_3_in_midair
        #else
        #define BUMPFALL_ACTION actions_5_bumped
        #endif

        // bumpfall
        SEQ_ACTION, BUMPFALL_ACTION, SEQ_DX, 1, SEQ_DY, 3, SEQ_JMP_IF_FEATHER, 0x25, 0x1E,
        frame_102_start_fall_1,
        SEQ_DX, 2, SEQ_DY, 6, frame_103_start_fall_2,
        SEQ_DX, (byte)-1, SEQ_DY, 9, frame_104_start_fall_3,
        SEQ_DY, 12, frame_105_start_fall_4,
        SEQ_DX, (byte)-2, SEQ_SET_FALL, 0, 15, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // bumpfloat
        frame_102_start_fall_1,
        SEQ_DX, 2, SEQ_DY, 3, frame_103_start_fall_2,
        SEQ_DX, (byte)-1, SEQ_DY, 4, frame_104_start_fall_3,
        SEQ_DY, 5, frame_105_start_fall_4,
        SEQ_DX, (byte)-2, SEQ_SET_FALL, 0, 6, SEQ_JMP, DW(0x1D49), // goto "freefall"

        // hard bump
        SEQ_ACTION, actions_5_bumped, SEQ_DX, (byte)-1, SEQ_DY, (byte)-4, frame_102_start_fall_1,
        SEQ_DX, (byte)-1, SEQ_DY, 3, SEQ_DX, (byte)-3, SEQ_DY, 1, SEQ_KNOCK_DOWN,
        SEQ_DX, 1, SEQ_SOUND, SND_FOOTSTEP, frame_107_fall_land_1,
        SEQ_DX, 2, frame_108_fall_land_2,
        SEQ_SOUND, SND_FOOTSTEP, frame_109_crouch,
        SEQ_JMP, DW(0x1FB3), // goto "standup"

        // test foot
        frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        frame_123_stepping_3,
        SEQ_DX, 2, frame_124_stepping_4,
        SEQ_DX, 4, frame_125_stepping_5,
        SEQ_DX, 3, frame_126_stepping_6,
        SEQ_DX, (byte)-4, frame_86_test_foot,
        SEQ_SOUND, SND_FOOTSTEP, SEQ_KNOCK_DOWN, SEQ_DX, (byte)-4, frame_116_stand_up_from_crouch_7,
        SEQ_DX, (byte)-2, frame_117_stand_up_from_crouch_8,
        frame_118_stand_up_from_crouch_9,
        frame_119_stand_up_from_crouch_10,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step back
        SEQ_DX, (byte)-5, SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 14 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 3, frame_124_stepping_4,
        SEQ_DX, 4, frame_125_stepping_5,
        SEQ_DX, 3, frame_126_stepping_6,
        SEQ_DX, (byte)-1, SEQ_DX, 3, frame_127_stepping_7,
        frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
        frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 13 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 3, frame_124_stepping_4,
        SEQ_DX, 4, frame_125_stepping_5,
        SEQ_DX, 3, frame_126_stepping_6,
        SEQ_DX, (byte)-1, SEQ_DX, 2, frame_127_stepping_7,
        frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
        frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 12 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 3, frame_124_stepping_4,
        SEQ_DX, 4, frame_125_stepping_5,
        SEQ_DX, 3, frame_126_stepping_6,
        SEQ_DX, (byte)-1, SEQ_DX, 1, frame_127_stepping_7,
        frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
        frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 11 pixels (normal step)
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 3, frame_124_stepping_4,
        SEQ_DX, 4, frame_125_stepping_5,
        SEQ_DX, 3, frame_126_stepping_6,
        SEQ_DX, (byte)-1, frame_127_stepping_7,
        frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
        frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 10 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, /* "step10a "*/ frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 3, frame_124_stepping_4,
        SEQ_DX, 4, frame_125_stepping_5,
        SEQ_DX, 3, frame_126_stepping_6,
        SEQ_DX, (byte)-2, frame_128_stepping_8,
        frame_129_stepping_9, frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 9 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_JMP, DW(0x1EFC), // goto "step10a"

        // step forward 8 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 3, frame_124_stepping_4,
        SEQ_DX, 4, frame_125_stepping_5,
        SEQ_DX, (byte)-1, frame_127_stepping_7,
        frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10, frame_131_stepping_11,
        frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 7 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 3, frame_124_stepping_4,
        SEQ_DX, 2, frame_129_stepping_9,
        frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 6 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 2, frame_124_stepping_4,
        SEQ_DX, 2, frame_129_stepping_9,
        frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 5 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 2, frame_124_stepping_4,
        SEQ_DX, 1, frame_129_stepping_9,
        frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 4 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 2, frame_131_stepping_11,
        frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 3 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_123_stepping_3,
        SEQ_DX, 1, frame_131_stepping_11,
        frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 2 pixels
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_122_stepping_2,
        SEQ_DX, 1, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // step forward 1 pixel
        SEQ_ACTION, actions_1_run_jump, frame_121_stepping_1,
        SEQ_DX, 1, frame_132_stepping_12,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // stoop
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, 1, frame_107_fall_land_1,
        SEQ_DX, 2, frame_108_fall_land_2,
        /* ":crouch" */frame_109_crouch,
        SEQ_JMP, DW(0x1FAF), // goto ":crouch"

        // stand up
        SEQ_ACTION, actions_5_bumped, SEQ_DX, 1, frame_110_stand_up_from_crouch_1,
        frame_111_stand_up_from_crouch_2,
        SEQ_DX, 2, frame_112_stand_up_from_crouch_3,
        frame_113_stand_up_from_crouch_4,
        SEQ_DX, 1, frame_114_stand_up_from_crouch_5,
        frame_115_stand_up_from_crouch_6, frame_116_stand_up_from_crouch_7,
        SEQ_DX, (byte)-4, frame_117_stand_up_from_crouch_8,
        frame_118_stand_up_from_crouch_9, frame_119_stand_up_from_crouch_10,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // pick up sword
        SEQ_ACTION, actions_1_run_jump, SEQ_GET_ITEM, 1, frame_229_found_sword,
        frame_229_found_sword, frame_229_found_sword, frame_229_found_sword, frame_229_found_sword,
        frame_229_found_sword, frame_230_sheathe, frame_231_sheathe, frame_232_sheathe,
        SEQ_JMP, DW(0x1FDA), // goto "resheathe"

        // resheathe
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, (byte)-5, frame_233_sheathe,
        frame_234_sheathe, frame_235_sheathe, frame_236_sheathe, frame_237_sheathe,
        frame_238_sheathe, frame_239_sheathe, frame_240_sheathe, frame_133_sheathe,
        frame_133_sheathe, frame_134_sheathe, frame_134_sheathe, frame_134_sheathe,
        frame_48_turn,
        SEQ_DX, 1, frame_49_turn,
        SEQ_DX, (byte)-2, SEQ_ACTION, actions_5_bumped, frame_50_turn,
        SEQ_ACTION, actions_1_run_jump, frame_51_turn,
        frame_52_turn,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // fast sheathe
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, (byte)-5, frame_234_sheathe,
        frame_236_sheathe, frame_238_sheathe, frame_240_sheathe, frame_134_sheathe,
        SEQ_DX, (byte)-1, SEQ_JMP, DW(0x19A0), // goto "stand"

        // drink potion
        SEQ_ACTION, actions_1_run_jump, SEQ_DX, 4, frame_191_drink,
        frame_192_drink, frame_193_drink, frame_194_drink, frame_195_drink,
        frame_196_drink, frame_197_drink, SEQ_SOUND, SND_DRINK,
        frame_198_drink, frame_199_drink, frame_200_drink, frame_201_drink,
        frame_202_drink, frame_203_drink, frame_204_drink, frame_205_drink,
        frame_205_drink, frame_205_drink,
        SEQ_GET_ITEM, 1, frame_205_drink,
        frame_205_drink, frame_201_drink, frame_198_drink,
        SEQ_DX, (byte)-4, SEQ_JMP, DW(0x19A0), // goto "stand"

        // soft land
        SEQ_ACTION, actions_5_bumped, SEQ_KNOCK_DOWN, SEQ_DX, 1, frame_107_fall_land_1,
        SEQ_DX, 2, frame_108_fall_land_2,
        SEQ_ACTION, actions_1_run_jump, /* ":crouch" */ frame_109_crouch,
        SEQ_JMP, DW(0x2036), // goto ":crouch"

        // land run
        SEQ_ACTION, actions_1_run_jump, SEQ_DY, (byte)-2, SEQ_DX, 1, frame_107_fall_land_1,
        SEQ_DX, 2, frame_108_fall_land_2,
        frame_109_crouch,
        SEQ_DX, 1, frame_110_stand_up_from_crouch_1,
        frame_111_stand_up_from_crouch_2,
        SEQ_DX, 2, frame_112_stand_up_from_crouch_3,
        frame_113_stand_up_from_crouch_4,
        SEQ_DX, 1, SEQ_DY, 1, frame_114_stand_up_from_crouch_5,
        SEQ_DY, 1, frame_115_stand_up_from_crouch_6,
        SEQ_DX, (byte)-2, SEQ_JMP, DW(0x1978), // goto "runstt4"

        // medium land (1.5 - 2 stories)
        SEQ_ACTION, actions_5_bumped, SEQ_KNOCK_DOWN, SEQ_DY, (byte)-2, SEQ_DX, 1, SEQ_DX, 2, frame_108_fall_land_2,
        frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
        frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
        frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
        frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
        frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
        frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
        frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
        frame_109_crouch,
        SEQ_DX, 1, frame_110_stand_up_from_crouch_1,
        frame_110_stand_up_from_crouch_1, frame_110_stand_up_from_crouch_1, frame_111_stand_up_from_crouch_2,
        SEQ_DX, 2, frame_112_stand_up_from_crouch_3,
        frame_113_stand_up_from_crouch_4,
        SEQ_DX, 1, SEQ_DY, 1, frame_114_stand_up_from_crouch_5,
        SEQ_DY, 1, frame_115_stand_up_from_crouch_6,
        frame_116_stand_up_from_crouch_7,
        SEQ_DX, (byte)-4, frame_117_stand_up_from_crouch_8,
        frame_118_stand_up_from_crouch_9, frame_119_stand_up_from_crouch_10,
        SEQ_JMP, DW(0x19A0), // goto "stand"

        // hard land (splat!; >2 stories)
        SEQ_ACTION, actions_5_bumped, SEQ_KNOCK_DOWN, SEQ_DY, (byte)-2, SEQ_DX, 3, frame_185_dead,
        SEQ_DIE, /* ":dead" */ frame_185_dead,
        SEQ_JMP, DW(0x20A5), // goto ":dead"

        // stabkill
        SEQ_ACTION, actions_5_bumped, SEQ_JMP, DW(0x20AE), // goto "dropdead"

        // dropdead
        SEQ_ACTION, actions_1_run_jump, SEQ_DIE, frame_179_collapse_1,
        frame_180_collapse_2, frame_181_collapse_3, frame_182_collapse_4,
        SEQ_DX, 1, frame_183_collapse_5,
        SEQ_DX, (byte)-4, /* ":dead" */ frame_185_dead,
        SEQ_JMP, DW(0x20BA), // goto ":dead"

        // impale
        SEQ_ACTION, actions_1_run_jump, SEQ_KNOCK_DOWN, SEQ_DX, 4, frame_177_spiked,
        SEQ_DIE, /* ":dead" */ frame_177_spiked,
        SEQ_JMP, DW(0x20C5), // goto ":dead"

        // halve
        SEQ_ACTION, actions_1_run_jump, frame_178_chomped,
        SEQ_DIE, /* ":dead" */ frame_178_chomped,
        SEQ_JMP, DW(0x20CD), // goto ":dead"

        // crush
        SEQ_JMP, DW(0x205A), // goto "medland"

        // deadfall
        SEQ_SET_FALL, 0, 0, SEQ_ACTION, actions_4_in_freefall, /* ":loop"*/ frame_185_dead,
        SEQ_JMP, DW(0x20D9), // goto ":loop"

        // climb stairs
        SEQ_ACTION, actions_5_bumped,
        SEQ_DX, (byte)-5, SEQ_DY, (byte)-1, SEQ_SOUND, SND_FOOTSTEP, frame_217_exit_stairs_1,
        frame_218_exit_stairs_2, frame_219_exit_stairs_3,
        SEQ_DX, 1, frame_220_exit_stairs_4,
        SEQ_DX, (byte)-4, SEQ_DY, (byte)-3, SEQ_SOUND, SND_FOOTSTEP, frame_221_exit_stairs_5,
        SEQ_DX, (byte)-4, SEQ_DY, (byte)-2, frame_222_exit_stairs_6,
        SEQ_DX, (byte)-2, SEQ_DY, (byte)-3, frame_223_exit_stairs_7,
        SEQ_DX, (byte)-3, SEQ_DY, (byte)-8, SEQ_SOUND, SND_LEVEL, SEQ_SOUND, SND_FOOTSTEP, frame_224_exit_stairs_8,
        SEQ_DX, (byte)-1, SEQ_DY, (byte)-1, frame_225_exit_stairs_9,
        SEQ_DX, (byte)-3, SEQ_DY, (byte)-4, frame_226_exit_stairs_10,
        SEQ_DX, (byte)-1, SEQ_DY, (byte)-5, SEQ_SOUND, SND_FOOTSTEP, frame_227_exit_stairs_11,
        SEQ_DX, (byte)-2, SEQ_DY, (byte)-1, frame_228_exit_stairs_12,
        frame_0,
        SEQ_SOUND, SND_FOOTSTEP, frame_0, frame_0, frame_0, // these footsteps are only heard when the music is off
        SEQ_SOUND, SND_FOOTSTEP, frame_0, frame_0, frame_0,
        SEQ_SOUND, SND_FOOTSTEP, frame_0, frame_0, frame_0,
        SEQ_SOUND, SND_FOOTSTEP, SEQ_END_LEVEL, /* ":loop" */ frame_0,
        SEQ_JMP, DW(0x212E), // goto ":loop"

        // Vizier: stand
        alt2frame_54_Vstand,
        SEQ_JMP, DW(0x2132), // goto "Vstand"

        // Vizier: raise arms
        85, 67, 67, 67, // numbers refer to frames in the "alternate" frame sets
        67, 67, 67, 67,
        67, 67, 67, 68,
        69, 70, 71, 72,
        73, 74, 75, 83,
        84, /* ":loop" */ 76,
        SEQ_JMP, DW(0x214B), // goto ":loop"

        // Vizier: walk
        SEQ_DX, 1, /* "Vwalk1" */ 48,
        SEQ_DX, 2, /* "Vwalk2" */ 49,
        SEQ_DX, 6, 50,
        SEQ_DX, 1, 51,
        SEQ_DX, (byte)-1, 52,
        SEQ_DX, 1, 53,
        SEQ_DX, 1, SEQ_JMP, DW(0x2151), // goto "Vwalk1"

        // Vizier: stop
        SEQ_DX, 1, 55,
        56,
        SEQ_JMP, DW(0x2132),

        // Vizier: lower arms, turn & exit ("Vexit")
        77, 78, 79, 80,
        81, 82,
        SEQ_DX, 1, 54,
        54, 54, 54, 54,
        54, 57, 58, 59,
        60, 61,
        SEQ_DX, 2, 62,
        SEQ_DX, (byte)-1, 63,
        SEQ_DX, (byte)-3, 64,
        65,
        SEQ_DX, (byte)-1, 66,
        SEQ_FLIP, SEQ_DX, 16, SEQ_DX, 3, SEQ_JMP, DW(0x2154), // goto "Vwalk2"

        // Princess: stand
        11,
        SEQ_JMP, DW(0x2195), // goto "Pstand"

        // Princess: alert
        2, 3, 4, 5,
        6, 7, 8, 9,
        SEQ_FLIP, SEQ_DX, 8, 11,
        SEQ_JMP, DW(0x2195),

        // Princess: step back
        SEQ_FLIP, SEQ_DX, 11, 12,
        SEQ_DX, 1, 13,
        SEQ_DX, 1, 14,
        SEQ_DX, 3, 15,
        SEQ_DX, 1, 16,
        /* ":loop" */ 17,
        SEQ_JMP, DW(0x21B8), // goto ":loop"

        // Princess lying on cushions ("Plie")
        19,
        SEQ_JMP, DW(0x21BC), // goto "Plie"

        // Princess: waiting
        /* ":loop" */ 20,
        SEQ_JMP, DW(0x21C0), // goto ":loop"

        // Princess: embrace
        21,
        SEQ_DX, 1, 22,
        23, 24,
        SEQ_DX, 1, 25,
        SEQ_DX, (byte)-3, 26,
        SEQ_DX, (byte)-2, 27,
        SEQ_DX, (byte)-4, 28,
        SEQ_DX, (byte)-3, 29,
        SEQ_DX, (byte)-2, 30,
        SEQ_DX, (byte)-3, 31,
        SEQ_DX, (byte)-1, 32,
        /* ":loop" */ 33,
        SEQ_JMP, DW(0x21E2), // goto ":loop"

        // Princess: stroke mouse
        /* ":loop" */ 37,
        SEQ_JMP, DW(0x21E6), // goto ":loop"

        // Princess: rise
        37, 38, 39, 40,
        41, 42, 43, 44,
        45, 46, 47,
        SEQ_FLIP, SEQ_DX, 12, /* ":loop" */ 11,
        SEQ_JMP, DW(0x21F8), // goto ":loop"

        // Princess: crouch & stroke mouse
        11, 11,
        SEQ_FLIP, SEQ_DX, 13, 47,
        46, 45, 44, 43,
        42, 41, 40, 39,
        38, 37,
        36, 36, 36,
        35, 35, 35,
        34, 34, 34, 34, 34, 34, 34,
        35, 35,
        36, 36, 36,
        35, 35, 35,
        34, 34, 34, 34, 34, 34, 34,
        35, 35,
        36, 36, 36,
        35, 35, 35,
        34, 34, 34, 34, 34, 34, 34, 34, 34,
        35, 35, 35,
        /* ":loop" */ 36,
        SEQ_JMP, DW(0x223C), // goto ":loop"

        // Princess: slump shoulders
        1, /* ":loop" */ 18,
        SEQ_JMP, DW(0x2241), // goto ":loop"

        // Mouse: scurry
        SEQ_ACTION, actions_1_run_jump, /* "Mscurry1" */ frame_186_mouse_1,
        SEQ_DX, 5, frame_186_mouse_1,
        SEQ_DX, 3, frame_187_mouse_2,
        SEQ_DX, 4, SEQ_JMP, DW(0x2247), // goto "Mscurry1"

        // Mouse: stop
        /* ":loop" */ frame_186_mouse_1,
        SEQ_JMP, DW(0x2253), // goto ":loop"

        // Mouse: raise head
        /* ":loop" */ frame_188_mouse_stand,
        SEQ_JMP, DW(0x2257), // goto ":loop"

        // Mouse: leave
        SEQ_ACTION, actions_0_stand, frame_186_mouse_1,
        frame_186_mouse_1, frame_186_mouse_1, frame_188_mouse_stand, frame_188_mouse_stand,
        frame_188_mouse_stand, frame_188_mouse_stand, frame_188_mouse_stand, frame_188_mouse_stand,
        frame_188_mouse_stand, frame_188_mouse_stand,
        SEQ_FLIP, SEQ_DX, 8, SEQ_JMP, DW(0x2247), // goto "Mscurry1"

        // Mouse: climb
        frame_186_mouse_1, frame_186_mouse_1, /* ":loop" */ frame_188_mouse_stand,
        SEQ_JMP, DW(0x2270) // goto ":loop"

};

#ifdef CHECK_SEQTABLE_MATCHES_ORIGINAL

// unmodified original sequence table
const byte original_seqtbl[] = {0xF9,0x01,0xFF,0x81,0x19,0xF9,0x01,0x01,0x02,0x03,0x04,0xFB,0x08,0x05,0xFB,0x03,0x06,0xFB,0x03,0x07,0xFB,0x05,0x08,0xFB,0x01,0xF2,0x01,0x09,0xFB,0x02,0x0A,0xFB,0x04,0x0B,0xFB,0x05,0x0C,0xFB,0x02,0xF2,0x01,0x0D,0xFB,0x03,0x0E,0xFB,0x04,0xFF,0x81,0x19,0xF9,0x00,0x0F,0xFF,0xA0,0x19,0xF9,0x01,0xA6,0xFF,0xA8,0x19,0xF9,0x05,0xFB,0x0A,0xB1,0xB1,0xFB,0xF9,0xFA,0xFE,0xB2,0xFB,0x05,0xFA,0x02,0xA6,0xFB,0xFF,0xFF,0xD2,0x19,0xFF,0xD2,0x19,0xF9,0x01,0xFB,0x02,0xCF,0xD0,0xFB,0x02,0xD1,0xFB,0x02,0xD2,0xFB,0x03,0xF9,0x01,0xF2,0x00,0x9E,0xAA,0xAB,0xFF,0xD8,0x19,0xF9,0x05,0xF8,0xFF,0x00,0xAC,0xFB,0xFF,0xFA,0x01,0xAD,0xFB,0xFF,0xAE,0xFB,0xFF,0xFA,0x02,0xFB,0xFE,0xFA,0x01,0xFB,0xFB,0xFA,0xFC,0xFF,0x4D,0x1A,0xF9,0x01,0xF8,0x01,0x00,0x9B,0xFB,0x02,0xA5,0xFB,0xFE,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0x9B,0x9C,0x9D,0x9E,0xFF,0x2E,0x1A,0xF9,0x01,0xF8,0x01,0x00,0xFB,0x02,0xA3,0xFB,0x04,0xA4,0xA5,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0x01,0x00,0xFB,0x06,0xA4,0xA5,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0xFB,0xFD,0xA0,0xFB,0xFE,0x9D,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0xA8,0xF9,0x01,0x97,0xF9,0x01,0x98,0x99,0x9A,0xF9,0x05,0x9B,0xF9,0x01,0x9C,0x9D,0xFF,0xD2,0x19,0xF9,0x01,0xA7,0xFF,0x4A,0x1A,0xA2,0xFF,0x45,0x1A,0xA9,0x96,0xFF,0xD2,0x19,0x9F,0xA0,0xFF,0x5F,0x1A,0xF9,0x01,0xF4,0xFF,0xD2,0x19,0xF9,0x05,0xFB,0xF8,0xFF,0xD2,0x19,0xF9,0x05,0xA0,0x9D,0xFF,0xD2,0x19,0xF9,0x07,0xFB,0xF8,0xFF,0x39,0x1B,0xF9,0x05,0xFE,0xFB,0x05,0xFF,0x2E,0x1A,0xF9,0x05,0xFE,0xFB,0x12,0xFF,0xA6,0x19,0xF9,0x01,0x10,0x11,0xFB,0x02,0x12,0xFB,0x02,0x13,0xFB,0x02,0x14,0xFB,0x02,0x15,0xFB,0x02,0x16,0xFB,0x07,0x17,0xFB,0x09,0x18,0xFB,0x05,0xFA,0xFA,0x19,0xFB,0x01,0xFA,0x06,0x1A,0xFB,0x04,0xF4,0xF2,0x01,0x1B,0xFB,0xFD,0x1C,0xFB,0x05,0x1D,0xF2,0x01,0x1E,0x1F,0x20,0x21,0xFB,0x01,0xFF,0xA0,0x19,0xF9,0x01,0xF2,0x01,0x22,0xFB,0x05,0x23,0xFB,0x06,0x24,0xFB,0x03,0x25,0xFB,0x05,0xF2,0x01,0x26,0xFB,0x07,0x27,0xFB,0x0C,0xFA,0xFD,0x28,0xFB,0x08,0xFA,0xF7,0x29,0xFB,0x08,0xFA,0xFE,0x2A,0xFB,0x04,0xFA,0x0B,0x2B,0xFB,0x04,0xFA,0x03,0x2C,0xFB,0x05,0xF4,0xF2,0x01,0xFF,0x81,0x19,0xF9,0x01,0xFB,0x01,0x6B,0xFB,0x02,0xFB,0x02,0x6C,0xFB,0x02,0x6D,0xFB,0x02,0x6D,0xFB,0x02,0x6D,0xFF,0x16,0x1B,0x00,0xF9,0x01,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0xFB,0x02,0x6C,0xFB,0x02,0x6D,0xFF,0x29,0x1B,0xF9,0x07,0xFE,0xFB,0x06,0x2D,0xFB,0x01,0x2E,0xFF,0xC4,0x19,0xF9,0x07,0xFE,0xFB,0x06,0x2D,0xFB,0x01,0x2E,0xFB,0x02,0x2F,0xFB,0xFF,0x30,0xFB,0x01,0x31,0xFB,0xFE,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0xFF,0xFF,0x75,0x19,0xF9,0x01,0xFB,0x01,0x35,0xFB,0x01,0xF2,0x01,0x36,0xFB,0x08,0x37,0xF2,0x01,0x38,0xFB,0x07,0x39,0xFB,0x03,0x3A,0xFB,0x01,0x3B,0x3C,0xFB,0x02,0x3D,0xFB,0xFF,0x3E,0x3F,0x40,0xFB,0xFF,0x41,0xFB,0xF2,0xFE,0xFF,0x95,0x19,0xF9,0x03,0xFA,0xFF,0x66,0xFB,0xFE,0xFA,0x06,0x67,0xFB,0xFE,0xFA,0x09,0x68,0xFB,0xFF,0xFA,0x0C,0x69,0xFB,0xFD,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFA,0xFF,0xFB,0xFE,0x66,0xFB,0xFD,0xFA,0x06,0x67,0xFB,0xFD,0xFA,0x09,0x68,0xFB,0xFE,0xFA,0x0C,0x69,0xFB,0xFD,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0xFF,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x01,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0xF7,0x06,0x1C,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x01,0x0F,0xFF,0x49,0x1D,0xFB,0xFF,0xFA,0xFD,0xFF,0xE4,0x1B,0xFB,0x01,0xFF,0xDB,0x1B,0x66,0xFB,0x02,0xFA,0x03,0x67,0xFB,0xFF,0xFA,0x04,0x68,0xFA,0x05,0x69,0xFB,0xFE,0xF8,0x01,0x06,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0x01,0xFA,0x09,0x68,0xFB,0x02,0xFA,0x0C,0x69,0xF8,0x02,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0x66,0xFB,0x03,0xFA,0x06,0x67,0xFB,0x02,0xFA,0x09,0x68,0xFB,0x03,0xFA,0x0C,0x69,0xF8,0x03,0x0F,0xFF,0x49,0x1D,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0xF9,0x02,0x4E,0x4F,0x50,0xFF,0xA1,0x1C,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0xF9,0x02,0xFB,0x01,0x4E,0xFB,0x02,0x4F,0xFB,0x01,0x50,0xFF,0xA1,0x1C,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0xFB,0xFF,0x4D,0xF9,0x02,0xFB,0xFE,0x4E,0xFB,0xFF,0x4F,0xFB,0xFF,0x50,0xFF,0xA1,0x1C,0xF9,0x02,0x5B,0x5A,0x59,0x58,0x57,0x57,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x61,0x60,0x5F,0x5E,0x5D,0x5C,0x5B,0x5A,0x59,0x58,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x5F,0x5E,0x5D,0x5C,0xFF,0x25,0x1D,0xF9,0x06,0x5C,0x5D,0x5D,0x5C,0x5C,0x5B,0xFF,0xD8,0x1C,0x87,0x88,0x89,0x89,0x8A,0x8A,0x8A,0x8A,0x89,0x88,0x87,0xFB,0xF9,0xFF,0x25,0x1D,0xF9,0x01,0x94,0x91,0x90,0x8F,0x8E,0x8D,0xFB,0xFB,0xFA,0x3F,0xFC,0xF9,0x03,0x8C,0x8A,0x88,0x5B,0xF9,0x02,0xFF,0xA4,0x1C,0xF9,0x01,0x87,0x88,0x89,0x8A,0x8B,0x8C,0xFB,0x05,0xFA,0xC1,0xFD,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0xF9,0x05,0x95,0xF9,0x01,0x76,0x77,0xFB,0x01,0xFF,0xA0,0x19,0x51,0x52,0xF9,0x05,0x53,0xF9,0x01,0xF4,0xF2,0x00,0x54,0x55,0xFB,0x03,0xFF,0xA0,0x19,0xF9,0x03,0x51,0xFA,0x06,0x51,0xFA,0x09,0x51,0xFA,0x0C,0xFB,0x02,0xF8,0x00,0x0C,0xFF,0x49,0x1D,0xF9,0x04,0x6A,0xFF,0x4B,0x1D,0xF9,0x01,0x35,0xFB,0x02,0xF2,0x01,0x36,0xFB,0x07,0x37,0xF2,0x01,0x38,0xFB,0x02,0x31,0xFB,0xFE,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0xF9,0x00,0xF5,0x4F,0xFF,0x25,0x1D,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0xFA,0xFC,0x4F,0xFA,0xFE,0x4F,0x4F,0xFA,0x02,0x4F,0xFA,0x04,0xFF,0x25,0x1D,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0xFA,0xFF,0x4D,0xFA,0xFD,0x4E,0xFA,0xFC,0x4F,0xFA,0xF6,0x4F,0xFA,0xF7,0x4F,0xFA,0xF8,0x4F,0xFA,0xF9,0x4F,0xFA,0xFA,0x4F,0xFA,0xFB,0x4F,0xFA,0xFC,0x4F,0xFA,0xFD,0x4F,0xFA,0xFE,0x4F,0xFA,0xFE,0x4F,0xFA,0xFF,0x4F,0xFA,0xFF,0x4F,0xFA,0xFF,0x4F,0x4F,0x4F,0x4F,0xFA,0x01,0x4F,0xFA,0x01,0x4F,0xFA,0x02,0x4F,0xFA,0x02,0x4F,0xFA,0x03,0x4F,0xFA,0x04,0x4F,0xFA,0x05,0x4F,0xFA,0x06,0x4F,0xF8,0x00,0x06,0xFF,0x49,0x1D,0xF9,0x03,0x50,0xFF,0xA1,0x1C,0xF9,0x05,0xFB,0xFC,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x05,0xFB,0x01,0xFA,0x03,0xF7,0x25,0x1E,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0x66,0xFB,0x02,0xFA,0x03,0x67,0xFB,0xFF,0xFA,0x04,0x68,0xFA,0x05,0x69,0xFB,0xFE,0xF8,0x00,0x06,0xFF,0x49,0x1D,0xF9,0x05,0xFB,0xFF,0xFA,0xFC,0x66,0xFB,0xFF,0xFA,0x03,0xFB,0xFD,0xFA,0x01,0xF4,0xFB,0x01,0xF2,0x01,0x6B,0xFB,0x02,0x6C,0xF2,0x01,0x6D,0xFF,0xB3,0x1F,0x79,0xFB,0x01,0x7A,0x7B,0xFB,0x02,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFC,0x56,0xF2,0x01,0xF4,0xFB,0xFC,0x74,0xFB,0xFE,0x75,0x76,0x77,0xFF,0xA0,0x19,0xFB,0xFB,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x03,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x02,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x01,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFE,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFF,0xFC,0x1E,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0xFF,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x02,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x7C,0xFB,0x02,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x7C,0xFB,0x01,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x01,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x84,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0x6D,0xFF,0xAF,0x1F,0xF9,0x05,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0x72,0x73,0x74,0xFB,0xFC,0x75,0x76,0x77,0xFF,0xA0,0x19,0xF9,0x01,0xF3,0x01,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE6,0xE7,0xE8,0xFF,0xDA,0x1F,0xF9,0x01,0xFB,0xFB,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0x85,0x85,0x86,0x86,0x86,0x30,0xFB,0x01,0x31,0xFB,0xFE,0xF9,0x05,0x32,0xF9,0x01,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0xFB,0xEA,0xEC,0xEE,0xF0,0x86,0xFB,0xFF,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0x04,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xF2,0x03,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCD,0xCD,0xF3,0x01,0xCD,0xCD,0xC9,0xC6,0xFB,0xFC,0xFF,0xA0,0x19,0xF9,0x05,0xF4,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0xF9,0x01,0x6D,0xFF,0x36,0x20,0xF9,0x01,0xFA,0xFE,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0x6D,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0xFA,0x01,0x72,0xFA,0x01,0x73,0xFB,0xFE,0xFF,0x78,0x19,0xF9,0x05,0xF4,0xFA,0xFE,0xFB,0x01,0xFB,0x02,0x6C,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0xFB,0x01,0x6E,0x6E,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0xFA,0x01,0x72,0xFA,0x01,0x73,0x74,0xFB,0xFC,0x75,0x76,0x77,0xFF,0xA0,0x19,0xF9,0x05,0xF4,0xFA,0xFE,0xFB,0x03,0xB9,0xF6,0xB9,0xFF,0xA5,0x20,0xF9,0x05,0xFF,0xAE,0x20,0xF9,0x01,0xF6,0xB3,0xB4,0xB5,0xB6,0xFB,0x01,0xB7,0xFB,0xFC,0xB9,0xFF,0xBA,0x20,0xF9,0x01,0xF4,0xFB,0x04,0xB1,0xF6,0xB1,0xFF,0xC5,0x20,0xF9,0x01,0xB2,0xF6,0xB2,0xFF,0xCD,0x20,0xFF,0x5A,0x20,0xF8,0x00,0x00,0xF9,0x04,0xB9,0xFF,0xD9,0x20,0xF9,0x05,0xFB,0xFB,0xFA,0xFF,0xF2,0x01,0xD9,0xDA,0xDB,0xFB,0x01,0xDC,0xFB,0xFC,0xFA,0xFD,0xF2,0x01,0xDD,0xFB,0xFC,0xFA,0xFE,0xDE,0xFB,0xFE,0xFA,0xFD,0xDF,0xFB,0xFD,0xFA,0xF8,0xF2,0x04,0xF2,0x01,0xE0,0xFB,0xFF,0xFA,0xFF,0xE1,0xFB,0xFD,0xFA,0xFC,0xE2,0xFB,0xFF,0xFA,0xFB,0xF2,0x01,0xE3,0xFB,0xFE,0xFA,0xFF,0xE4,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0xF1,0x00,0xFF,0x2E,0x21,0x36,0xFF,0x32,0x21,0x55,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x53,0x54,0x4C,0xFF,0x4B,0x21,0xFB,0x01,0x30,0xFB,0x02,0x31,0xFB,0x06,0x32,0xFB,0x01,0x33,0xFB,0xFF,0x34,0xFB,0x01,0x35,0xFB,0x01,0xFF,0x51,0x21,0xFB,0x01,0x37,0x38,0xFF,0x32,0x21,0x4D,0x4E,0x4F,0x50,0x51,0x52,0xFB,0x01,0x36,0x36,0x36,0x36,0x36,0x36,0x39,0x3A,0x3B,0x3C,0x3D,0xFB,0x02,0x3E,0xFB,0xFF,0x3F,0xFB,0xFD,0x40,0x41,0xFB,0xFF,0x42,0xFE,0xFB,0x10,0xFB,0x03,0xFF,0x54,0x21,0x0B,0xFF,0x95,0x21,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFE,0xFB,0x08,0x0B,0xFF,0x95,0x21,0xFE,0xFB,0x0B,0x0C,0xFB,0x01,0x0D,0xFB,0x01,0x0E,0xFB,0x03,0x0F,0xFB,0x01,0x10,0x11,0xFF,0xB8,0x21,0x13,0xFF,0xBC,0x21,0x14,0xFF,0xC0,0x21,0x15,0xFB,0x01,0x16,0x17,0x18,0xFB,0x01,0x19,0xFB,0xFD,0x1A,0xFB,0xFE,0x1B,0xFB,0xFC,0x1C,0xFB,0xFD,0x1D,0xFB,0xFE,0x1E,0xFB,0xFD,0x1F,0xFB,0xFF,0x20,0x21,0xFF,0xE2,0x21,0x25,0xFF,0xE6,0x21,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0xFE,0xFB,0x0C,0x0B,0xFF,0xF8,0x21,0x0B,0x0B,0xFE,0xFB,0x0D,0x2F,0x2E,0x2D,0x2C,0x2B,0x2A,0x29,0x28,0x27,0x26,0x25,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x23,0x24,0xFF,0x3C,0x22,0x01,0x12,0xFF,0x41,0x22,0xF9,0x01,0xBA,0xFB,0x05,0xBA,0xFB,0x03,0xBB,0xFB,0x04,0xFF,0x47,0x22,0xBA,0xFF,0x53,0x22,0xBC,0xFF,0x57,0x22,0xF9,0x00,0xBA,0xBA,0xBA,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xFE,0xFB,0x08,0xFF,0x47,0x22,0xBA,0xBA,0xBC,0xFF,0x70,0x22};

void check_seqtable_matches_original() {
        printf("Checking that the sequence table matches the original DOS version...\n");
        int different = 0;
        int i;
        for(i = 0; i < 2310; ++i) {
                if (seqtbl[i] != original_seqtbl[i]) {
                        different = 1;
                        printf("Seqtbl difference at index %d (%#x; shifted offset %#x): value is %d, should be %d\n"
                                , i, i, i + 0x196E, seqtbl[i], original_seqtbl[i]);
                }
        }
        if (!different) printf("All good, no differences found!\n");
}

#endif // CHECK_SEQTABLE_MATCHES_ORIGINAL
