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

#define SEQTBL_BASE 0x196E
#define SEQTBL_0 (seqtbl - SEQTBL_BASE)

// This expands a two-byte number into two comma-separated bytes, used for the JMP destinations
#define DW(data_word) (data_word) & 0x00FF, (((data_word) & 0xFF00) >> 8)

// Shorter notation for the sequence table instructions
#define act(action) SEQ_ACTION, action
#define jmp(dest) SEQ_JMP, DW(dest)
#define jmp_if_feather(dest) SEQ_JMP_IF_FEATHER, DW(dest)
#define dx(amount) SEQ_DX, (byte) amount
#define dy(amount) SEQ_DY, (byte) amount
#define snd(sound) SEQ_SOUND, sound
#define set_fall(x, y) SEQ_SET_FALL, (byte) x, (byte) y

// This splits the byte array into labeled "sections" that are packed tightly next to each other
// However, it only seems to work correctly in the Debug configuration...
//#define LABEL(label) }; const byte label##_eventual_ptr[] __attribute__ ((aligned(1))) = {
#define LABEL(label) // disable
//#define OFFSET(label) label - seqtbl + SEQTBL_BASE

// Labels
#define running             SEQTBL_BASE                                   // 0x196E
#define startrun            5  + running            //SEQTBL_BASE + 5     // 0x1973
#define runstt1             2  + startrun           //SEQTBL_BASE + 7     // 0x1975
#define runstt4             3  + runstt1            //SEQTBL_BASE + 10    // 0x1978
#define runcyc1             9  + runstt4            //SEQTBL_BASE + 19    // 0x1981
#define runcyc7             20 + runcyc1            //SEQTBL_BASE + 39    // 0x1995
#define stand               11 + runcyc7            //SEQTBL_BASE + 50    // 0x19A0
#define goalertstand        6  + stand              //SEQTBL_BASE + 56    // 0x19A6
#define alertstand          2  + goalertstand       //SEQTBL_BASE + 58    // 0x19A8
#define arise               4  + alertstand         //SEQTBL_BASE + 62    // 0x19AC
#define guardengarde        21 + arise              //SEQTBL_BASE + 83    // 0x19C1
#define engarde             3  + guardengarde       //SEQTBL_BASE + 86    // 0x19C4
#define ready               14 + engarde            //SEQTBL_BASE + 100   // 0x19D2
#define ready_loop          6  + ready              //SEQTBL_BASE + 106   // 0x19D8
#define stabbed             4  + ready_loop         //SEQTBL_BASE + 110   // 0x19DC
#define strikeadv           29 + stabbed            //SEQTBL_BASE + 139   // 0x19F9
#define strikeret           14 + strikeadv          //SEQTBL_BASE + 153   // 0x1A07
#define advance             12 + strikeret          //SEQTBL_BASE + 165   // 0x1A13
#define fastadvance         15 + advance            //SEQTBL_BASE + 180   // 0x1A22
#define retreat             12 + fastadvance        //SEQTBL_BASE + 192   // 0x1A2E
#define strike              14 + retreat            //SEQTBL_BASE + 206   // 0x1A3C
#define faststrike          6  + strike             //SEQTBL_BASE + 212   // 0x1A42
#define guy4                3  + faststrike         //SEQTBL_BASE + 215   // 0x1A45
#define guy7                5  + guy4               //SEQTBL_BASE + 220   // 0x1A4A
#define guy8                3  + guy7               //SEQTBL_BASE + 223   // 0x1A4D
#define blockedstrike       7  + guy8               //SEQTBL_BASE + 230   // 0x1A54
#define blocktostrike       6  + blockedstrike      //SEQTBL_BASE + 236   // 0x1A5A
#define readyblock          4  + blocktostrike      //SEQTBL_BASE + 240   // 0x1A5E
#define blocking            1  + readyblock         //SEQTBL_BASE + 241   // 0x1A5F
#define striketoblock       4  + blocking           //SEQTBL_BASE + 245   // 0x1A63
#define landengarde         5  + striketoblock      //SEQTBL_BASE + 250   // 0x1A68
#define bumpengfwd          6  + landengarde        //SEQTBL_BASE + 256   // 0x1A6E
#define bumpengback         7  + bumpengfwd         //SEQTBL_BASE + 263   // 0x1A75
#define flee                7  + bumpengback        //SEQTBL_BASE + 270   // 0x1A7C
#define turnengarde         7  + flee               //SEQTBL_BASE + 277   // 0x1A83
#define alertturn           8  + turnengarde        //SEQTBL_BASE + 285   // 0x1A8B
#define standjump           8  + alertturn          //SEQTBL_BASE + 293   // 0x1A93
#define sjland              29 + standjump          //SEQTBL_BASE + 322   // 0x1AB0
#define runjump             29 + sjland             //SEQTBL_BASE + 351   // 0x1ACD
#define rjlandrun           46 + runjump            //SEQTBL_BASE + 397   // 0x1AFB
#define rdiveroll           9  + rjlandrun          //SEQTBL_BASE + 406   // 0x1B04
#define rdiveroll_crouch    18 + rdiveroll          //SEQTBL_BASE + 424   // 0x1B16
#define sdiveroll           4  + rdiveroll_crouch   //SEQTBL_BASE + 428   // 0x1B1A
#define crawl               1  + sdiveroll          //SEQTBL_BASE + 429   // 0x1B1B
#define crawl_crouch        14 + crawl              //SEQTBL_BASE + 443   // 0x1B29
#define turndraw            4  + crawl_crouch       //SEQTBL_BASE + 447   // 0x1B2D
#define turn                12 + turndraw           //SEQTBL_BASE + 459   // 0x1B39
#define turnrun             26 + turn               //SEQTBL_BASE + 485   // 0x1B53
#define runturn             7  + turnrun            //SEQTBL_BASE + 492   // 0x1B5A
#define fightfall           43 + runturn            //SEQTBL_BASE + 535   // 0x1B85
#define efightfall          28 + fightfall          //SEQTBL_BASE + 563   // 0x1BA1
#define efightfallfwd       30 + efightfall         //SEQTBL_BASE + 593   // 0x1BBF
#define stepfall            28 + efightfallfwd      //SEQTBL_BASE + 621   // 0x1BDB
#define fall1               9  + stepfall           //SEQTBL_BASE + 630   // 0x1BE4
#define patchfall           22 + fall1              //SEQTBL_BASE + 652   // 0x1BFA
#define stepfall2           7  + patchfall          //SEQTBL_BASE + 659   // 0x1C01
#define stepfloat           5  + stepfall2          //SEQTBL_BASE + 664   // 0x1C06
#define jumpfall            22 + stepfloat          //SEQTBL_BASE + 686   // 0x1C1C
#define rjumpfall           28 + jumpfall           //SEQTBL_BASE + 714   // 0x1C38
#define jumphangMed         28 + rjumpfall          //SEQTBL_BASE + 742   // 0x1C54
#define jumphangLong        21 + jumphangMed        //SEQTBL_BASE + 763   // 0x1C69
#define jumpbackhang        27 + jumphangLong       //SEQTBL_BASE + 790   // 0x1C84
#define hang                29 + jumpbackhang       //SEQTBL_BASE + 819   // 0x1CA1
#define hang1               3  + hang               //SEQTBL_BASE + 822   // 0x1CA4
#define hangstraight        45 + hang1              //SEQTBL_BASE + 867   // 0x1CD1
#define hangstraight_loop   7  + hangstraight       //SEQTBL_BASE + 874   // 0x1CD8
#define climbfail           4  + hangstraight_loop  //SEQTBL_BASE + 878   // 0x1CDC
#define climbdown           16 + climbfail          //SEQTBL_BASE + 894   // 0x1CEC
#define climbup             24 + climbdown          //SEQTBL_BASE + 918   // 0x1D04
#define hangdrop            33 + climbup            //SEQTBL_BASE + 951   // 0x1D25
#define hangfall            17 + hangdrop           //SEQTBL_BASE + 968   // 0x1D36
#define freefall            19 + hangfall           //SEQTBL_BASE + 987   // 0x1D49
#define freefall_loop       2  + freefall           //SEQTBL_BASE + 989   // 0x1D4B
#define runstop             4  + freefall_loop      //SEQTBL_BASE + 993   // 0x1D4F
#define jumpup              25 + runstop            //SEQTBL_BASE + 1018  // 0x1D68
#define highjump            21 + jumpup             //SEQTBL_BASE + 1039  // 0x1D7D
#define superhijump         30 + highjump           //SEQTBL_BASE + 1069  // 0x1D9B
#define fallhang            91 + superhijump        //SEQTBL_BASE + 1160  // 0x1DF6
#define bump                6  + fallhang           //SEQTBL_BASE + 1166  // 0x1DFC
#define bumpfall            10 + bump               //SEQTBL_BASE + 1176  // 0x1E06
#define bumpfloat           31 + bumpfall           //SEQTBL_BASE + 1207  // 0x1E25
#define hardbump            22 + bumpfloat          //SEQTBL_BASE + 1229  // 0x1E3B
#define testfoot            30 + hardbump           //SEQTBL_BASE + 1259  // 0x1E59
#define stepback            31 + testfoot           //SEQTBL_BASE + 1290  // 0x1E78
#define step14              5  + stepback           //SEQTBL_BASE + 1295  // 0x1E7D
#define step13              31 + step14             //SEQTBL_BASE + 1326  // 0x1E9C
#define step12              31 + step13             //SEQTBL_BASE + 1357  // 0x1EBB
#define step11              31 + step12             //SEQTBL_BASE + 1388  // 0x1EDA
#define step10              29 + step11             //SEQTBL_BASE + 1417  // 0x1EF7
#define step10a             5  + step10             //SEQTBL_BASE + 1422  // 0x1EFC
#define step9               23 + step10a            //SEQTBL_BASE + 1445  // 0x1F13
#define step8               6  + step9              //SEQTBL_BASE + 1451  // 0x1F19
#define step7               26 + step8              //SEQTBL_BASE + 1477  // 0x1F33
#define step6               21 + step7              //SEQTBL_BASE + 1498  // 0x1F48
#define step5               21 + step6              //SEQTBL_BASE + 1519  // 0x1F5D
#define step4               21 + step5              //SEQTBL_BASE + 1540  // 0x1F72
#define step3               16 + step4              //SEQTBL_BASE + 1556  // 0x1F82
#define step2               16 + step3              //SEQTBL_BASE + 1572  // 0x1F92
#define step1               12 + step2              //SEQTBL_BASE + 1584  // 0x1F9E
#define stoop               9  + step1              //SEQTBL_BASE + 1593  // 0x1FA7
#define stoop_crouch        8  + stoop              //SEQTBL_BASE + 1601  // 0x1FAF
#define standup             4  + stoop_crouch       //SEQTBL_BASE + 1605  // 0x1FB3
#define pickupsword         23 + standup            //SEQTBL_BASE + 1628  // 0x1FCA
#define resheathe           16 + pickupsword        //SEQTBL_BASE + 1644  // 0x1FDA
#define fastsheathe         33 + resheathe          //SEQTBL_BASE + 1677  // 0x1FFB
#define drinkpotion         14 + fastsheathe        //SEQTBL_BASE + 1691  // 0x2009
#define softland            34 + drinkpotion        //SEQTBL_BASE + 1725  // 0x202B
#define softland_crouch     11 + softland           //SEQTBL_BASE + 1736  // 0x2036
#define landrun             4  + softland_crouch    //SEQTBL_BASE + 1740  // 0x203A
#define medland             32 + landrun            //SEQTBL_BASE + 1772  // 0x205A
#define hardland            66 + medland            //SEQTBL_BASE + 1838  // 0x209C
#define hardland_dead       9  + hardland           //SEQTBL_BASE + 1847  // 0x20A5
#define stabkill            4  + hardland_dead      //SEQTBL_BASE + 1851  // 0x20A9
#define dropdead            5  + stabkill           //SEQTBL_BASE + 1856  // 0x20AE
#define dropdead_dead       12 + dropdead           //SEQTBL_BASE + 1868  // 0x20BA
#define impale              4  + dropdead_dead      //SEQTBL_BASE + 1872  // 0x20BE
#define impale_dead         7  + impale             //SEQTBL_BASE + 1879  // 0x20C5
#define halve               4  + impale_dead        //SEQTBL_BASE + 1883  // 0x20C9
#define halve_dead          4  + halve              //SEQTBL_BASE + 1887  // 0x20CD
#define crush               4  + halve_dead         //SEQTBL_BASE + 1891  // 0x20D1
#define deadfall            3  + crush              //SEQTBL_BASE + 1894  // 0x20D4
#define deadfall_loop       5  + deadfall           //SEQTBL_BASE + 1899  // 0x20D9
#define climbstairs         4  + deadfall_loop      //SEQTBL_BASE + 1903  // 0x20DD
#define climbstairs_loop    81 + climbstairs        //SEQTBL_BASE + 1984  // 0x212E
#define Vstand              4  + climbstairs_loop   //SEQTBL_BASE + 1988  // 0x2132
#define Vraise              4  + Vstand             //SEQTBL_BASE + 1992  // 0x2136
#define Vraise_loop         21 + Vraise             //SEQTBL_BASE + 2013  // 0x214B
#define Vwalk               4  + Vraise_loop        //SEQTBL_BASE + 2017  // 0x214F
#define Vwalk1              2  + Vwalk              //SEQTBL_BASE + 2019  // 0x2151
#define Vwalk2              3  + Vwalk1             //SEQTBL_BASE + 2022  // 0x2154
#define Vstop               18 + Vwalk2             //SEQTBL_BASE + 2040  // 0x2166
#define Vexit               7  + Vstop              //SEQTBL_BASE + 2047  // 0x216D
#define Pstand              40 + Vexit              //SEQTBL_BASE + 2087  // 0x2195
#define Palert              4  + Pstand             //SEQTBL_BASE + 2091  // 0x2199
#define Pstepback           15 + Palert             //SEQTBL_BASE + 2106  // 0x21A8
#define Pstepback_loop      16 + Pstepback          //SEQTBL_BASE + 2122  // 0x21B8
#define Plie                4  + Pstepback_loop     //SEQTBL_BASE + 2126  // 0x21BC
#define Pwaiting            4  + Plie               //SEQTBL_BASE + 2130  // 0x21C0
#define Pembrace            4  + Pwaiting           //SEQTBL_BASE + 2134  // 0x21C4
#define Pembrace_loop       30 + Pembrace           //SEQTBL_BASE + 2164  // 0x21E2
#define Pstroke             4  + Pembrace_loop      //SEQTBL_BASE + 2168  // 0x21E6
#define Prise               4  + Pstroke            //SEQTBL_BASE + 2172  // 0x21EA
#define Prise_loop          14 + Prise              //SEQTBL_BASE + 2186  // 0x21F8
#define Pcrouch             4  + Prise_loop         //SEQTBL_BASE + 2190  // 0x21FC
#define Pcrouch_loop        64 + Pcrouch            //SEQTBL_BASE + 2254  // 0x223C
#define Pslump              4  + Pcrouch_loop       //SEQTBL_BASE + 2258  // 0x2240
#define Pslump_loop         1  + Pslump             //SEQTBL_BASE + 2259  // 0x2241
#define Mscurry             4  + Pslump_loop        //SEQTBL_BASE + 2263  // 0x2245
#define Mscurry1            2  + Mscurry            //SEQTBL_BASE + 2265  // 0x2247
#define Mstop               12 + Mscurry1           //SEQTBL_BASE + 2277  // 0x2253
#define Mraise              4  + Mstop              //SEQTBL_BASE + 2281  // 0x2257
#define Mleave              4  + Mraise             //SEQTBL_BASE + 2285  // 0x225B
#define Mclimb              19 + Mleave             //SEQTBL_BASE + 2304  // 0x226E
#define Mclimb_loop         2  + Mclimb             //SEQTBL_BASE + 2306  // 0x2270

const word seqtbl_offsets[] = {
        0x0000,         startrun,       stand,          standjump,
        runjump,        turn,           runturn,        stepfall,
        jumphangMed,    hang,           climbup,        hangdrop,
        freefall,       runstop,        jumpup,         fallhang,
        jumpbackhang,   softland,       jumpfall,       stepfall2,
        medland,        rjumpfall,      hardland,       hangfall,
        jumphangLong,   hangstraight,   rdiveroll,      sdiveroll,
        highjump,       step1,          step2,          step3,
        step4,          step5,          step6,          step7,
        step8,          step9,          step10,         step11,
        step12,         step13,         step14,         turnrun,
        testfoot,       bumpfall,       hardbump,       bump,
        superhijump,    standup,        stoop,          impale,
        crush,          deadfall,       halve,          engarde,
        advance,        retreat,        strike,         flee,
        turnengarde,    striketoblock,  readyblock,     landengarde,
        bumpengfwd,     bumpengback,    blocktostrike,  strikeadv,
        climbdown,      blockedstrike,  climbstairs,    dropdead,
        stepback,       climbfail,      stabbed,        faststrike,
        strikeret,      alertstand,     drinkpotion,    crawl,
        alertturn,      fightfall,      efightfall,     efightfallfwd,
        running,        stabkill,       fastadvance,    goalertstand,
        arise,          turndraw,       guardengarde,   pickupsword,
        resheathe,      fastsheathe,    Pstand,         Vstand,
        Vwalk,          Vstop,          Palert,         Pstepback,
        Vexit,          Mclimb,         Vraise,         Plie,
        patchfall,      Mscurry,        Mstop,          Mleave,
        Pembrace,       Pwaiting,       Pstroke,        Prise,
        Pcrouch,        Pslump,         Mraise
};

// data:196E
byte seqtbl[] = {

	LABEL(running) // running
	act(actions_1_run_jump), jmp(runcyc1), // goto running: frame 7

	LABEL(startrun) // startrun
	act(actions_1_run_jump), LABEL(runstt1) frame_1_start_run,
	frame_2_start_run, frame_3_start_run, LABEL(runstt4) frame_4_start_run,
	dx(8), frame_5_start_run,
	dx(3), frame_6_start_run,
	dx(3), LABEL(runcyc1) frame_7_run,
	dx(5), frame_8_run,
	dx(1), snd(SND_FOOTSTEP), frame_9_run,
	dx(2), frame_10_run,
	dx(4), frame_11_run,
	dx(5), frame_12_run,
	dx(2), LABEL(runcyc7) snd(SND_FOOTSTEP), frame_13_run,
	dx(3), frame_14_run,
	dx(4), jmp(runcyc1),

	LABEL(stand) // stand
	act(actions_0_stand), frame_15_stand,
	jmp(stand), // goto "stand"

	LABEL(goalertstand) // alert stand
	act(actions_1_run_jump), LABEL(alertstand) frame_166_stand_inactive,
	jmp(alertstand), // goto "alertstand"

	LABEL(arise) // arise (skeleton)
	act(actions_5_bumped), dx(10), frame_177_spiked,
	frame_177_spiked,
	dx(-7), dy(-2), frame_178_chomped,
	dx(5), dy(2), frame_166_stand_inactive,
	dx(-1), jmp(ready), // goto "ready"

	// guard engarde
	LABEL(guardengarde)
	jmp(ready), // goto "ready"

	// engarde
	LABEL(engarde)
	act(actions_1_run_jump),
	dx(2), frame_207_draw_1,
	frame_208_draw_2,
	dx(2), frame_209_draw_3,
	dx(2), frame_210_draw_4,
	dx(3), LABEL(ready) act(actions_1_run_jump), snd(SND_SILENT), frame_158_stand_with_sword,
	frame_170_stand_with_sword,
	LABEL(ready_loop) frame_171_stand_with_sword,
	jmp(ready_loop), // goto ":loop"

	LABEL(stabbed)// stabbed
	act(actions_5_bumped), set_fall(-1, 0), frame_172_jumpfall_2,
	dx(-1), dy(1), frame_173_jumpfall_3,
	dx(-1), frame_174_jumpfall_4,
	dx(-1), dy(2), // frame 175 is commented out in the Apple II source
	dx(-2), dy(1),
	dx(-5), dy(-4),
	jmp(guy8), // goto "guy8"

	// strike - advance
	LABEL(strikeadv)
	act(actions_1_run_jump), set_fall(1, 0), frame_155_guy_7,
	dx(2), frame_165_walk_with_sword,
	dx(-2), jmp(ready), // goto "ready"

	LABEL(strikeret)// strike - retreat
	act(actions_1_run_jump), set_fall(-1, 0), frame_155_guy_7,
	frame_156_guy_8,
	frame_157_walk_with_sword,
	frame_158_stand_with_sword,
	jmp(retreat), // goto "retreat"

	LABEL(advance) // advance
	act(actions_1_run_jump), set_fall(1, 0),
	dx(2), frame_163_fighting,
	dx(4), frame_164_fighting,
	frame_165_walk_with_sword,
	jmp(ready), // goto "ready"

	LABEL(fastadvance) // fast advance
	act(actions_1_run_jump), set_fall(1, 0), dx(6), frame_164_fighting,
	frame_165_walk_with_sword,
	jmp(ready), // goto "ready"

	LABEL(retreat) // retreat
	act(actions_1_run_jump), set_fall(-1, 0), dx(-3), frame_160_fighting,
	dx(-2), frame_157_walk_with_sword,
	jmp(ready), // goto "ready"

	LABEL(strike) // strike
	act(actions_1_run_jump), set_fall(-1, 0), frame_168_back,
	LABEL(faststrike) act(actions_1_run_jump), frame_151_strike_1,
	LABEL(guy4) act(actions_1_run_jump), frame_152_strike_2,
	frame_153_strike_3,
	frame_154_poking,
	LABEL(guy7) act(actions_5_bumped), frame_155_guy_7,
	LABEL(guy8) act(actions_1_run_jump), frame_156_guy_8,
	frame_157_walk_with_sword,
	jmp(ready), // goto "ready"

	LABEL(blockedstrike)// blocked strike
	act(actions_1_run_jump), frame_167_blocked,
	jmp(guy7), // goto "guy7"

	// block to strike
	LABEL(blocktostrike)// "blocktostrike"
	frame_162_block_to_strike,
	jmp(guy4), // goto "guy4"

	LABEL(readyblock) // ready block
	frame_169_begin_block,
	LABEL(blocking) frame_150_parry,
	jmp(ready), // goto "ready"

	LABEL(striketoblock) // strike to block
	frame_159_fighting,
	frame_160_fighting,
	jmp(blocking), // goto "blocking"

	LABEL(landengarde) // land en garde
	act(actions_1_run_jump), SEQ_KNOCK_DOWN, jmp(ready), // goto "ready"

	LABEL(bumpengfwd) // bump en garde (forward)
	act(actions_5_bumped), dx(-8), jmp(ready), // goto "ready"

	LABEL(bumpengback) // bump en garde (back)
	act(actions_5_bumped), frame_160_fighting,
	frame_157_walk_with_sword,
	jmp(ready), // goto "ready"

	LABEL(flee) // flee
	act(actions_7_turn), dx(-8), jmp(turn), // goto "turn"

	LABEL(turnengarde) // turn en garde
	act(actions_5_bumped), SEQ_FLIP, dx(5), jmp(retreat), // goto "retreat"

	LABEL(alertturn) // alert turn (for enemies)
	act(actions_5_bumped), SEQ_FLIP, dx(18), jmp(goalertstand), // goto "goalertstand"

	LABEL(standjump) // standing jump
	act(actions_1_run_jump), frame_16_standing_jump_1,
	frame_17_standing_jump_2,
	dx(2), frame_18_standing_jump_3,
	dx(2), frame_19_standing_jump_4,
	dx(2), frame_20_standing_jump_5,
	dx(2), frame_21_standing_jump_6,
	dx(2), frame_22_standing_jump_7,
	dx(7), frame_23_standing_jump_8,
	dx(9), frame_24_standing_jump_9,
	dx(5), dy(-6), /* "sjland" */ LABEL(sjland) frame_25_standing_jump_10,
	dx(1), dy(6), frame_26_standing_jump_11,
	dx(4), SEQ_KNOCK_DOWN, snd(SND_FOOTSTEP), frame_27_standing_jump_12,
	dx(-3), frame_28_standing_jump_13,
	dx(5), frame_29_standing_jump_14,
	snd(SND_FOOTSTEP), frame_30_standing_jump_15,
	frame_31_standing_jump_16,
	frame_32_standing_jump_17,
	frame_33_standing_jump_18,
	dx(1), jmp(stand), // goto "stand"

	LABEL(runjump) // running jump
	act(actions_1_run_jump), snd(SND_FOOTSTEP), frame_34_start_run_jump_1,
	dx(5), frame_35_start_run_jump_2,
	dx(6), frame_36_start_run_jump_3,
	dx(3), frame_37_start_run_jump_4,
	dx(5), snd(SND_FOOTSTEP), frame_38_start_run_jump_5,
	dx(7), frame_39_start_run_jump_6,
	dx(12), dy(-3), frame_40_running_jump_1,
	dx(8), dy(-9), frame_41_running_jump_2,
	dx(8), dy(-2), frame_42_running_jump_3,
	dx(4), dy(11), frame_43_running_jump_4,
	dx(4), dy(3), /* "rjlandrun" */ LABEL(rjlandrun) frame_44_running_jump_5,
	dx(5), SEQ_KNOCK_DOWN, snd(SND_FOOTSTEP), jmp(runcyc1), // goto "runcyc1"

	LABEL(rdiveroll) // run dive roll
	act(actions_1_run_jump), dx(1), frame_107_fall_land_1,
	dx(2), dx(2), frame_108_fall_land_2,
	dx(2), frame_109_crouch,
	dx(2), frame_109_crouch,
	dx(2), /* ":crouch" */ LABEL(rdiveroll_crouch) frame_109_crouch,
	jmp(rdiveroll_crouch), // goto ":crouch"

	LABEL(sdiveroll)
	0x00, // stand dive roll; not implemented

	LABEL(crawl) // crawl
	act(actions_1_run_jump), dx(1), frame_110_stand_up_from_crouch_1,
	frame_111_stand_up_from_crouch_2,
	dx(2), frame_112_stand_up_from_crouch_3,
	dx(2), frame_108_fall_land_2,
	dx(2), /* ":crouch" */ LABEL(crawl_crouch) frame_109_crouch,
	jmp(crawl_crouch), // goto ":crouch"

	LABEL(turndraw) // turn draw
	act(actions_7_turn), SEQ_FLIP, dx(6), frame_45_turn,
	dx(1), frame_46_turn,
	jmp(engarde), // goto "engarde"

	LABEL(turn) // turn
	act(actions_7_turn), SEQ_FLIP, dx(6), frame_45_turn,
	dx(1), frame_46_turn,
	dx(2), frame_47_turn,
	dx(-1), /* "finishturn" */ frame_48_turn,
	dx(1), frame_49_turn,
	dx(-2), frame_50_turn,
	frame_51_turn,
	frame_52_turn,
	jmp(stand), // goto "stand"

	LABEL(turnrun) // turnrun (from frame 48)
	act(actions_1_run_jump), dx(-1), jmp(runstt1), // goto "runstt1"

	LABEL(runturn) // runturn
	act(actions_1_run_jump), dx(1), frame_53_runturn,
	dx(1), snd(SND_FOOTSTEP), frame_54_runturn,
	dx(8) ,frame_55_runturn,
	snd(SND_FOOTSTEP), frame_56_runturn,
	dx(7), frame_57_runturn,
	dx(3), frame_58_runturn,
	dx(1), frame_59_runturn,
	frame_60_runturn,
	dx(2), frame_61_runturn,
	dx(-1), frame_62_runturn,
	frame_63_runturn,
	frame_64_runturn,
	dx(-1), frame_65_runturn,
	dx(-14), SEQ_FLIP, jmp(runcyc7), // goto "runcyc7"

	LABEL(fightfall) // fightfall (backward)
	act(actions_3_in_midair), dy(-1), frame_102_start_fall_1,
	dx(-2), dy(6), frame_103_start_fall_2,
	dx(-2), dy(9), frame_104_start_fall_3,
	dx(-1), dy(12), frame_105_start_fall_4,
	dx(-3), set_fall(0, 15), jmp(freefall), // goto "freefall"

	LABEL(efightfall) // enemy fight fall
	act(actions_3_in_midair), dy(-1), dx(-2), frame_102_start_fall_1,
	dx(-3), dy(6), frame_103_start_fall_2,
	dx(-3), dy(9), frame_104_start_fall_3,
	dx(-2), dy(12), frame_105_start_fall_4,
	dx(-3), set_fall(0, 15), jmp(freefall), // goto "freefall"

	LABEL(efightfallfwd)// enemy fight fall forward
	act(actions_3_in_midair), dx(1), dy(-1), frame_102_start_fall_1,
	dx(2), dy(6), frame_103_start_fall_2,
	dx(-1), dy(9), frame_104_start_fall_3,
	dy(12), frame_105_start_fall_4,
	dx(-2), set_fall(1, 15), jmp(freefall), // goto "freefall"

	LABEL(stepfall) // stepfall
	act(actions_3_in_midair), dx(1), dy(3), jmp_if_feather(stepfloat), // goto "stepfloat"
	/* "fall1" */ LABEL(fall1) frame_102_start_fall_1,
	dx(2), dy(6), frame_103_start_fall_2,
	dx(-1), dy(9), frame_104_start_fall_3,
	dy(12), frame_105_start_fall_4,
	dx(-2), set_fall(1, 15), jmp(freefall), // goto "freefall"

	LABEL(patchfall) // patchfall
	dx(-1), dy(-3), jmp(fall1), // goto "fall1"

	LABEL(stepfall2) // stepfall2 (from frame 12)
	dx(1), jmp(stepfall), // goto "stepfall"

	LABEL(stepfloat) // stepfloat
	frame_102_start_fall_1,
	dx(2), dy(3), frame_103_start_fall_2,
	dx(-1), dy(4), frame_104_start_fall_3,
	dy(5), frame_105_start_fall_4,
	dx(-2), set_fall(1, 6), jmp(freefall), // goto "freefall"

	LABEL(jumpfall) // jump fall (from standing jump)
	act(actions_3_in_midair), dx(1), dy(3), frame_102_start_fall_1,
	dx(2), dy(6), frame_103_start_fall_2,
	dx(1), dy(9), frame_104_start_fall_3,
	dx(2), dy(12), frame_105_start_fall_4,
	set_fall(2, 15), jmp(freefall), // goto "freefall"

	LABEL(rjumpfall) // running jump fall
	act(actions_3_in_midair), dx(1), dy(3), frame_102_start_fall_1,
	dx(3), dy(6), frame_103_start_fall_2,
	dx(2), dy(9), frame_104_start_fall_3,
	dx(3), dy(12), frame_105_start_fall_4,
	set_fall(3, 15), jmp(freefall), // goto "freefall"

	LABEL(jumphangMed) // jumphang (medium: DX = 0)
	act(actions_1_run_jump), frame_67_start_jump_up_1,
	frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
	frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang, frame_76_jumphang,
	frame_77_jumphang,
	act(actions_2_hang_climb), frame_78_jumphang, frame_79_jumphang, frame_80_jumphang,
	jmp(hang), // goto "hang"

	LABEL(jumphangLong)// jumphang (long: DX = 4)
	act(actions_1_run_jump), frame_67_start_jump_up_1,
	frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
	frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang, frame_76_jumphang,
	frame_77_jumphang,
	act(actions_2_hang_climb), dx(1), frame_78_jumphang,
	dx(2), frame_79_jumphang,
	dx(1), frame_80_jumphang,
	jmp(hang), // goto "hang"

	LABEL(jumpbackhang) // jumpbackhang
	act(actions_1_run_jump), frame_67_start_jump_up_1,
	frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
	frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang, frame_76_jumphang,
	dx(-1), frame_77_jumphang,
	act(actions_2_hang_climb), dx(-2), frame_78_jumphang,
	dx(-1), frame_79_jumphang,
	dx(-1), frame_80_jumphang,
	jmp(hang), // goto "hang"

	LABEL(hang) // hang
	act(actions_2_hang_climb), frame_91_hanging_5,
	/* "hang1" */ LABEL(hang1) frame_90_hanging_4, frame_89_hanging_3, frame_88_hanging_2,
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
	jmp(hangdrop), // goto "hangdrop"

	LABEL(hangstraight) // hangstraight
	act(actions_6_hang_straight), frame_92_hanging_6, // Apple II source has a bump sound here
	frame_93_hanging_7, frame_93_hanging_7, frame_92_hanging_6, frame_92_hanging_6,
	/* ":loop" */ LABEL(hangstraight_loop) frame_91_hanging_5,
	jmp(hangstraight_loop), // goto ":loop"

	LABEL(climbfail) // climbfail
	frame_135_climbing_1, frame_136_climbing_2, frame_137_climbing_3, frame_137_climbing_3,
	frame_138_climbing_4, frame_138_climbing_4, frame_138_climbing_4, frame_138_climbing_4,
	frame_137_climbing_3, frame_136_climbing_2, frame_135_climbing_1,
	dx(-7), jmp(hangdrop), // goto "hangdrop"

	LABEL(climbdown) // climbdown
	act(actions_1_run_jump), frame_148_climbing_14,
	frame_145_climbing_11, frame_144_climbing_10, frame_143_climbing_9, frame_142_climbing_8,
	frame_141_climbing_7,
	dx(-5), dy(63), SEQ_DOWN, act(actions_3_in_midair), frame_140_climbing_6,
	frame_138_climbing_4, frame_136_climbing_2,
	frame_91_hanging_5,
	act(actions_2_hang_climb), jmp(hang1), // goto "hang1"

	LABEL(climbup) // climbup
	act(actions_1_run_jump), frame_135_climbing_1,
	frame_136_climbing_2, frame_137_climbing_3, frame_138_climbing_4,
	frame_139_climbing_5, frame_140_climbing_6,
	dx(5), dy(-63), SEQ_UP, frame_141_climbing_7,
	frame_142_climbing_8, frame_143_climbing_9, frame_144_climbing_10, frame_145_climbing_11,
	frame_146_climbing_12, frame_147_climbing_13, frame_148_climbing_14,
	act(actions_5_bumped), // to clear flags
	frame_149_climbing_15,
	act(actions_1_run_jump), frame_118_stand_up_from_crouch_9, frame_119_stand_up_from_crouch_10,
	dx(1), jmp(stand), // goto "stand"

	LABEL(hangdrop) // hangdrop
	frame_81_hangdrop_1, frame_82_hangdrop_2,
	act(actions_5_bumped), frame_83_hangdrop_3,
	act(actions_1_run_jump), SEQ_KNOCK_DOWN, snd(SND_SILENT),
	frame_84_hangdrop_4, frame_85_hangdrop_5,
	dx(3), jmp(stand), // goto "stand"

	LABEL(hangfall) // hangfall
	act(actions_3_in_midair), frame_81_hangdrop_1,
	dy(6), frame_81_hangdrop_1,
	dy(9), frame_81_hangdrop_1,
	dy(12), dx(2), set_fall(0, 12), jmp(freefall), // goto "freefall"

	LABEL(freefall) // freefall
	act(actions_4_in_freefall), /* ":loop" */ LABEL(freefall_loop) frame_106_fall,
	jmp(freefall_loop), // goto :loop

	LABEL(runstop) // runstop
	act(actions_1_run_jump), frame_53_runturn,
	dx(2), snd(SND_FOOTSTEP), frame_54_runturn,
	dx(7), frame_55_runturn,
	snd(SND_FOOTSTEP), frame_56_runturn,
	dx(2), frame_49_turn,
	dx(-2), frame_50_turn,
	frame_51_turn, frame_52_turn,
	jmp(stand), // goto "stand"

	LABEL(jumpup) // jump up (and touch ceiling)
	act(actions_1_run_jump), frame_67_start_jump_up_1,
	frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
	frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang,
	frame_76_jumphang, frame_77_jumphang, frame_78_jumphang,
	act(actions_0_stand), SEQ_KNOCK_UP, frame_79_jumphang,
	jmp(hangdrop), // goto "hangdrop"

	LABEL(highjump) // highjump (no ceiling above)
	act(actions_1_run_jump), frame_67_start_jump_up_1,
	frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang, frame_71_jumphang,
	frame_72_jumphang, frame_73_jumphang, frame_74_jumphang, frame_75_jumphang,
	frame_76_jumphang, frame_77_jumphang, frame_78_jumphang, frame_79_jumphang,
	dy(-4), frame_79_jumphang,
	dy(-2), frame_79_jumphang,
	frame_79_jumphang,
	dy(2), frame_79_jumphang,
	dy(4), jmp(hangdrop), // goto "hangdrop"

	LABEL(superhijump) // superhijump (when weightless)
	frame_67_start_jump_up_1, frame_68_start_jump_up_2, frame_69_start_jump_up_3, frame_70_jumphang,
	frame_71_jumphang, frame_72_jumphang, frame_73_jumphang, frame_74_jumphang,
	frame_75_jumphang, frame_76_jumphang,
	dy(-1), frame_77_jumphang,
	dy(-3), frame_78_jumphang,
	dy(-4), frame_79_jumphang,
	dy(-10), frame_79_jumphang,
	dy(-9), frame_79_jumphang,
	dy(-8), frame_79_jumphang,
	dy(-7), frame_79_jumphang,
	dy(-6), frame_79_jumphang,
	dy(-5), frame_79_jumphang,
	dy(-4), frame_79_jumphang,
	dy(-3), frame_79_jumphang,
	dy(-2), frame_79_jumphang,
	dy(-2), frame_79_jumphang,
	dy(-1), frame_79_jumphang,
	dy(-1), frame_79_jumphang,
	dy(-1), frame_79_jumphang,
	frame_79_jumphang, frame_79_jumphang, frame_79_jumphang,
	dy(1), frame_79_jumphang,
	dy(1), frame_79_jumphang,
	dy(2), frame_79_jumphang,
	dy(2), frame_79_jumphang,
	dy(3), frame_79_jumphang,
	dy(4), frame_79_jumphang,
	dy(5), frame_79_jumphang,
	dy(6), frame_79_jumphang,
	set_fall(0, 6), jmp(freefall), // goto "freefall"

	LABEL(fallhang) // fall hang
	act(actions_3_in_midair), frame_80_jumphang,
	jmp(hang), // goto "hang"

	LABEL(bump) // bump
	act(actions_5_bumped), dx(-4), frame_50_turn,
	frame_51_turn, frame_52_turn,
	jmp(stand), // goto "stand"

	LABEL(bumpfall) // bumpfall
	/* action is patched to 3_in_midair by FIX_WALLBUMP_TRIGGERS_TILE_BELOW */
	act(actions_5_bumped), dx(1), dy(3), jmp_if_feather(bumpfloat),
	frame_102_start_fall_1,
	dx(2), dy(6), frame_103_start_fall_2,
	dx(-1), dy(9), frame_104_start_fall_3,
	dy(12), frame_105_start_fall_4,
	dx(-2), set_fall(0, 15), jmp(freefall), // goto "freefall"

	LABEL(bumpfloat) // bumpfloat
	frame_102_start_fall_1,
	dx(2), dy(3), frame_103_start_fall_2,
	dx(-1), dy(4), frame_104_start_fall_3,
	dy(5), frame_105_start_fall_4,
	dx(-2), set_fall(0, 6), jmp(freefall), // goto "freefall"

	LABEL(hardbump) // hard bump
	act(actions_5_bumped), dx(-1), dy(-4), frame_102_start_fall_1,
	dx(-1), dy(3), dx(-3), dy(1), SEQ_KNOCK_DOWN,
	dx(1), snd(SND_FOOTSTEP), frame_107_fall_land_1,
	dx(2), frame_108_fall_land_2,
	snd(SND_FOOTSTEP), frame_109_crouch,
	jmp(standup), // goto "standup"

	LABEL(testfoot) // test foot
	frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	frame_123_stepping_3,
	dx(2), frame_124_stepping_4,
	dx(4), frame_125_stepping_5,
	dx(3), frame_126_stepping_6,
	dx(-4), frame_86_test_foot,
	snd(SND_FOOTSTEP), SEQ_KNOCK_DOWN, dx(-4), frame_116_stand_up_from_crouch_7,
	dx(-2), frame_117_stand_up_from_crouch_8,
	frame_118_stand_up_from_crouch_9,
	frame_119_stand_up_from_crouch_10,
	jmp(stand), // goto "stand"

	LABEL(stepback) // step back
	dx(-5), jmp(stand), // goto "stand"

	LABEL(step14) // step forward 14 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(3), frame_124_stepping_4,
	dx(4), frame_125_stepping_5,
	dx(3), frame_126_stepping_6,
	dx(-1), dx(3), frame_127_stepping_7,
	frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
	frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step13) // step forward 13 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(3), frame_124_stepping_4,
	dx(4), frame_125_stepping_5,
	dx(3), frame_126_stepping_6,
	dx(-1), dx(2), frame_127_stepping_7,
	frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
	frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step12) // step forward 12 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(3), frame_124_stepping_4,
	dx(4), frame_125_stepping_5,
	dx(3), frame_126_stepping_6,
	dx(-1), dx(1), frame_127_stepping_7,
	frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
	frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step11) // step forward 11 pixels (normal step)
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(3), frame_124_stepping_4,
	dx(4), frame_125_stepping_5,
	dx(3), frame_126_stepping_6,
	dx(-1), frame_127_stepping_7,
	frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10,
	frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step10) // step forward 10 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), /* "step10a "*/ LABEL(step10a) frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(3), frame_124_stepping_4,
	dx(4), frame_125_stepping_5,
	dx(3), frame_126_stepping_6,
	dx(-)2, frame_128_stepping_8,
	frame_129_stepping_9, frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step9) // step forward 9 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	jmp(step10a), // goto "step10a"

	LABEL(step8) // step forward 8 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(3), frame_124_stepping_4,
	dx(4), frame_125_stepping_5,
	dx(-1), frame_127_stepping_7,
	frame_128_stepping_8, frame_129_stepping_9, frame_130_stepping_10, frame_131_stepping_11,
	frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step7) // step forward 7 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(3), frame_124_stepping_4,
	dx(2), frame_129_stepping_9,
	frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step6) // step forward 6 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(2), frame_124_stepping_4,
	dx(2), frame_129_stepping_9,
	frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step5) // step forward 5 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(2), frame_124_stepping_4,
	dx(1), frame_129_stepping_9,
	frame_130_stepping_10, frame_131_stepping_11, frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step4) // step forward 4 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(2), frame_131_stepping_11,
	frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step3) // step forward 3 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_123_stepping_3,
	dx(1), frame_131_stepping_11,
	frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step2) // step forward 2 pixels
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_122_stepping_2,
	dx(1), frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(step1) // step forward 1 pixel
	act(actions_1_run_jump), frame_121_stepping_1,
	dx(1), frame_132_stepping_12,
	jmp(stand), // goto "stand"

	LABEL(stoop) // stoop
	act(actions_1_run_jump), dx(1), frame_107_fall_land_1,
	dx(2), frame_108_fall_land_2,
	/* ":crouch" */ LABEL(stoop_crouch) frame_109_crouch,
	jmp(stoop_crouch), // goto ":crouch

	LABEL(standup) // stand up
	act(actions_5_bumped), dx(1), frame_110_stand_up_from_crouch_1,
	frame_111_stand_up_from_crouch_2,
	dx(2), frame_112_stand_up_from_crouch_3,
	frame_113_stand_up_from_crouch_4,
	dx(1 /*patched to 0 by FIX_STAND_ON_THIN_AIR*/), frame_114_stand_up_from_crouch_5,
	frame_115_stand_up_from_crouch_6, frame_116_stand_up_from_crouch_7,
	dx(-4 /*patched to -3 by FIX_STAND_ON_THIN_AIR*/), frame_117_stand_up_from_crouch_8,
	frame_118_stand_up_from_crouch_9, frame_119_stand_up_from_crouch_10,
	jmp(stand), // goto "stand"

	LABEL(pickupsword) // pick up sword
	act(actions_1_run_jump), SEQ_GET_ITEM, 1, frame_229_found_sword,
	frame_229_found_sword, frame_229_found_sword, frame_229_found_sword, frame_229_found_sword,
	frame_229_found_sword, frame_230_sheathe, frame_231_sheathe, frame_232_sheathe,
	jmp(resheathe), // goto "resheathe"

	LABEL(resheathe) // resheathe
	act(actions_1_run_jump), dx(-5), frame_233_sheathe,
	frame_234_sheathe, frame_235_sheathe, frame_236_sheathe, frame_237_sheathe,
	frame_238_sheathe, frame_239_sheathe, frame_240_sheathe, frame_133_sheathe,
	frame_133_sheathe, frame_134_sheathe, frame_134_sheathe, frame_134_sheathe,
	frame_48_turn,
	dx(1), frame_49_turn,
	dx(-2), act(actions_5_bumped), frame_50_turn,
	act(actions_1_run_jump), frame_51_turn,
	frame_52_turn,
	jmp(stand), // goto "stand"

	LABEL(fastsheathe) // fast sheathe
	act(actions_1_run_jump), dx(-5), frame_234_sheathe,
	frame_236_sheathe, frame_238_sheathe, frame_240_sheathe, frame_134_sheathe,
	dx(-1), jmp(stand), // goto "stand"

	LABEL(drinkpotion) // drink potion
	act(actions_1_run_jump), dx(4), frame_191_drink,
	frame_192_drink, frame_193_drink, frame_194_drink, frame_195_drink,
	frame_196_drink, frame_197_drink, snd(SND_DRINK),
	frame_198_drink, frame_199_drink, frame_200_drink, frame_201_drink,
	frame_202_drink, frame_203_drink, frame_204_drink, frame_205_drink,
	frame_205_drink, frame_205_drink,
	SEQ_GET_ITEM, 1, frame_205_drink,
	frame_205_drink, frame_201_drink, frame_198_drink,
	dx(-4), jmp(stand), // goto "stand"

	LABEL(softland)// soft land
	act(actions_5_bumped), SEQ_KNOCK_DOWN, dx(1), frame_107_fall_land_1,
	dx(2), frame_108_fall_land_2,
	act(actions_1_run_jump), /* ":crouch" */ LABEL(softland_crouch) frame_109_crouch,
	jmp(softland_crouch), // goto ":crouch"

	LABEL(landrun) // land run
	act(actions_1_run_jump), dy(-2), dx(1), frame_107_fall_land_1,
	dx(2), frame_108_fall_land_2,
	frame_109_crouch,
	dx(1), frame_110_stand_up_from_crouch_1,
	frame_111_stand_up_from_crouch_2,
	dx(2), frame_112_stand_up_from_crouch_3,
	frame_113_stand_up_from_crouch_4,
	dx(1), dy(1), frame_114_stand_up_from_crouch_5,
	dy(1), frame_115_stand_up_from_crouch_6,
	dx(-2), jmp(runstt4), // goto "runstt4"

	LABEL(medland) // medium land (1.5 - 2 stories)
	act(actions_5_bumped), SEQ_KNOCK_DOWN, dy(-2), dx(1), dx(2), frame_108_fall_land_2,
	frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
	frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
	frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
	frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
	frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
	frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
	frame_109_crouch, frame_109_crouch, frame_109_crouch, frame_109_crouch,
	frame_109_crouch,
	dx(1), frame_110_stand_up_from_crouch_1,
	frame_110_stand_up_from_crouch_1, frame_110_stand_up_from_crouch_1, frame_111_stand_up_from_crouch_2,
	dx(2), frame_112_stand_up_from_crouch_3,
	frame_113_stand_up_from_crouch_4,
	dx(1), dy(1), frame_114_stand_up_from_crouch_5,
	dy(1), frame_115_stand_up_from_crouch_6,
	frame_116_stand_up_from_crouch_7,
	dx(-4), frame_117_stand_up_from_crouch_8,
	frame_118_stand_up_from_crouch_9, frame_119_stand_up_from_crouch_10,
	jmp(stand), // goto "stand"

	LABEL(hardland) // hard land (splat!; >2 stories)
	act(actions_5_bumped), SEQ_KNOCK_DOWN, dy(-2), dx(3), frame_185_dead,
	SEQ_DIE, /* ":dead" */ LABEL(hardland_dead) frame_185_dead,
	jmp(hardland_dead), // goto ":dead"

	LABEL(stabkill) // stabkill
	act(actions_5_bumped), jmp(dropdead), // goto "dropdead"

	LABEL(dropdead) // dropdead
	act(actions_1_run_jump), SEQ_DIE, frame_179_collapse_1,
	frame_180_collapse_2, frame_181_collapse_3, frame_182_collapse_4,
	dx(1), frame_183_collapse_5,
	dx(-4), /* ":dead" */ LABEL(dropdead_dead) frame_185_dead,
	jmp(dropdead_dead), // goto ":dead"

	LABEL(impale) // impale
	act(actions_1_run_jump), SEQ_KNOCK_DOWN, dx(4), frame_177_spiked,
	SEQ_DIE, /* ":dead" */ LABEL(impale_dead) frame_177_spiked,
	jmp(impale_dead), // goto ":dead"

	LABEL(halve) // halve
	act(actions_1_run_jump), frame_178_chomped,
	SEQ_DIE, /* ":dead" */ LABEL(halve_dead) frame_178_chomped,
	jmp(halve_dead), // goto ":dead"

	LABEL(crush) // crush
	jmp(medland), // goto "medland"

	LABEL(deadfall) // deadfall
	set_fall(0, 0), act(actions_4_in_freefall), /* ":loop"*/ LABEL(deadfall_loop) frame_185_dead,
	jmp(deadfall_loop), // goto ":loop"

	LABEL(climbstairs) // climb stairs
	act(actions_5_bumped),
	dx(-5), dy(-1), snd(SND_FOOTSTEP), frame_217_exit_stairs_1,
	frame_218_exit_stairs_2, frame_219_exit_stairs_3,
	dx(1), frame_220_exit_stairs_4,
	dx(-4), dy(-3), snd(SND_FOOTSTEP), frame_221_exit_stairs_5,
	dx(-4), dy(-2), frame_222_exit_stairs_6,
	dx(-2), dy(-3), frame_223_exit_stairs_7,
	dx(-3), dy(-8), snd(SND_LEVEL), snd(SND_FOOTSTEP), frame_224_exit_stairs_8,
	dx(-1), dy(-1), frame_225_exit_stairs_9,
	dx(-3), dy(-4), frame_226_exit_stairs_10,
	dx(-1), dy(-5), snd(SND_FOOTSTEP), frame_227_exit_stairs_11,
	dx(-2), dy(-1), frame_228_exit_stairs_12,
	frame_0,
	snd(SND_FOOTSTEP), frame_0, frame_0, frame_0, // these footsteps are only heard when the music is off
	snd(SND_FOOTSTEP), frame_0, frame_0, frame_0,
	snd(SND_FOOTSTEP), frame_0, frame_0, frame_0,
	snd(SND_FOOTSTEP), SEQ_END_LEVEL, /* ":loop" */ LABEL(climbstairs_loop) frame_0,
	jmp(climbstairs_loop), // goto ":loop"

	LABEL(Vstand) // Vizier: stand
	alt2frame_54_Vstand,
	jmp(Vstand), // goto "Vstand"

	LABEL(Vraise) // Vizier: raise arms
	85, 67, 67, 67, // numbers refer to frames in the "alternate" frame sets
	67, 67, 67, 67,
	67, 67, 67, 68,
	69, 70, 71, 72,
	73, 74, 75, 83,
	84, /* ":loop" */ LABEL(Vraise_loop) 76,
	jmp(Vraise_loop), // goto ":loop"

	LABEL(Vwalk) // Vizier: walk
	dx(1), /* "Vwalk1" */ LABEL(Vwalk1) 48,
	dx(2), /* "Vwalk2" */ LABEL(Vwalk2) 49,
	dx(6), 50,
	dx(1), 51,
	dx(-1), 52,
	dx(1), 53,
	dx(1), jmp(Vwalk1), // goto "Vwalk1"

	LABEL(Vstop) // Vizier: stop
	dx(1), 55,
	56,
	jmp(Vstand),

	LABEL(Vexit) // Vizier: lower arms, turn & exit ("Vexit")
	77, 78, 79, 80,
	81, 82,
	dx(1), 54,
	54, 54, 54, 54,
	54, 57, 58, 59,
	60, 61,
	dx(2), 62,
	dx(-1), 63,
	dx(-3), 64,
	65,
	dx(-1), 66,
	SEQ_FLIP, dx(16), dx(3), jmp(Vwalk2), // goto "Vwalk2"

	// Princess: stand
	LABEL(Pstand) 11,
	jmp(Pstand), // goto "Pstand"

	LABEL(Palert) // Princess: alert
	2, 3, 4, 5,
	6, 7, 8, 9,
	SEQ_FLIP, dx(8), 11,
	jmp(Pstand),

	LABEL(Pstepback) // Princess: step back
	SEQ_FLIP, dx(11), 12,
	dx(1), 13,
	dx(1), 14,
	dx(3), 15,
	dx(1), 16,
	/* ":loop" */ LABEL(Pstepback_loop) 17,
	jmp(Pstepback_loop), // goto ":loop"

	LABEL(Plie) // Princess lying on cushions ("Plie")
	19,
	jmp(Plie), // goto "Plie"

	LABEL(Pwaiting) // Princess: waiting
	/* ":loop" */ 20,
	jmp(Pwaiting), // goto ":loop"

	LABEL(Pembrace)// Princess: embrace
	21,
	dx(1), 22,
	23, 24,
	dx(1), 25,
	dx(-3), 26,
	dx(-2), 27,
	dx(-4), 28,
	dx(-3), 29,
	dx(-2), 30,
	dx(-3), 31,
	dx(-1), 32,
	/* ":loop" */ LABEL(Pembrace_loop) 33,
	jmp(Pembrace_loop), // goto ":loop"

	LABEL(Pstroke) // Princess: stroke mouse
	/* ":loop" */ 37,
	jmp(Pstroke), // goto ":loop"

	LABEL(Prise) // Princess: rise
	37, 38, 39, 40,
	41, 42, 43, 44,
	45, 46, 47,
	SEQ_FLIP, dx(12), /* ":loop" */ LABEL(Prise_loop) 11,
	jmp(Prise_loop), // goto ":loop"

	LABEL(Pcrouch) // Princess: crouch & stroke mouse
	11, 11,
	SEQ_FLIP, dx(13), 47,
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
	/* ":loop" */ LABEL(Pcrouch_loop) 36,
	jmp(Pcrouch_loop), // goto ":loop"

	LABEL(Pslump) // Princess: slump shoulders
	1, /* ":loop" */ LABEL(Pslump_loop) 18,
	jmp(Pslump_loop), // goto ":loop"

	LABEL(Mscurry) // Mouse: scurry
	act(actions_1_run_jump), /* "Mscurry1" */ LABEL(Mscurry1) frame_186_mouse_1,
	dx(5), frame_186_mouse_1,
	dx(3), frame_187_mouse_2,
	dx(4), jmp(Mscurry1), // goto "Mscurry1"

	LABEL(Mstop) // Mouse: stop
	/* ":loop" */ frame_186_mouse_1,
	jmp(Mstop), // goto ":loop"

	LABEL(Mraise) // Mouse: raise head
	/* ":loop" */ frame_188_mouse_stand,
	jmp(Mraise), // goto ":loop"

	LABEL(Mleave) // Mouse: leave
	act(actions_0_stand), frame_186_mouse_1,
	frame_186_mouse_1, frame_186_mouse_1, frame_188_mouse_stand, frame_188_mouse_stand,
	frame_188_mouse_stand, frame_188_mouse_stand, frame_188_mouse_stand, frame_188_mouse_stand,
	frame_188_mouse_stand, frame_188_mouse_stand,
	SEQ_FLIP, dx(8), jmp(Mscurry1), // goto "Mscurry1"

	LABEL(Mclimb) // Mouse: climb
	frame_186_mouse_1, frame_186_mouse_1, /* ":loop" */ LABEL(Mclimb_loop) frame_188_mouse_stand,
	jmp(Mclimb_loop) // goto ":loop"

};

void apply_seqtbl_patches() {
#ifdef FIX_WALL_BUMP_TRIGGERS_TILE_BELOW
	if (fix_wall_bump_triggers_tile_below)
		SEQTBL_0[bumpfall + 1] = actions_3_in_midair; // instead of actions_5_bumped
#endif
}

#ifdef CHECK_SEQTABLE_MATCHES_ORIGINAL

// unmodified original sequence table
const byte original_seqtbl[] = {0xF9,0x01,0xFF,0x81,0x19,0xF9,0x01,0x01,0x02,0x03,0x04,0xFB,0x08,0x05,0xFB,0x03,0x06,0xFB,0x03,0x07,0xFB,0x05,0x08,0xFB,0x01,0xF2,0x01,0x09,0xFB,0x02,0x0A,0xFB,0x04,0x0B,0xFB,0x05,0x0C,0xFB,0x02,0xF2,0x01,0x0D,0xFB,0x03,0x0E,0xFB,0x04,0xFF,0x81,0x19,0xF9,0x00,0x0F,0xFF,0xA0,0x19,0xF9,0x01,0xA6,0xFF,0xA8,0x19,0xF9,0x05,0xFB,0x0A,0xB1,0xB1,0xFB,0xF9,0xFA,0xFE,0xB2,0xFB,0x05,0xFA,0x02,0xA6,0xFB,0xFF,0xFF,0xD2,0x19,0xFF,0xD2,0x19,0xF9,0x01,0xFB,0x02,0xCF,0xD0,0xFB,0x02,0xD1,0xFB,0x02,0xD2,0xFB,0x03,0xF9,0x01,0xF2,0x00,0x9E,0xAA,0xAB,0xFF,0xD8,0x19,0xF9,0x05,0xF8,0xFF,0x00,0xAC,0xFB,0xFF,0xFA,0x01,0xAD,0xFB,0xFF,0xAE,0xFB,0xFF,0xFA,0x02,0xFB,0xFE,0xFA,0x01,0xFB,0xFB,0xFA,0xFC,0xFF,0x4D,0x1A,0xF9,0x01,0xF8,0x01,0x00,0x9B,0xFB,0x02,0xA5,0xFB,0xFE,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0x9B,0x9C,0x9D,0x9E,0xFF,0x2E,0x1A,0xF9,0x01,0xF8,0x01,0x00,0xFB,0x02,0xA3,0xFB,0x04,0xA4,0xA5,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0x01,0x00,0xFB,0x06,0xA4,0xA5,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0xFB,0xFD,0xA0,0xFB,0xFE,0x9D,0xFF,0xD2,0x19,0xF9,0x01,0xF8,0xFF,0x00,0xA8,0xF9,0x01,0x97,0xF9,0x01,0x98,0x99,0x9A,0xF9,0x05,0x9B,0xF9,0x01,0x9C,0x9D,0xFF,0xD2,0x19,0xF9,0x01,0xA7,0xFF,0x4A,0x1A,0xA2,0xFF,0x45,0x1A,0xA9,0x96,0xFF,0xD2,0x19,0x9F,0xA0,0xFF,0x5F,0x1A,0xF9,0x01,0xF4,0xFF,0xD2,0x19,0xF9,0x05,0xFB,0xF8,0xFF,0xD2,0x19,0xF9,0x05,0xA0,0x9D,0xFF,0xD2,0x19,0xF9,0x07,0xFB,0xF8,0xFF,0x39,0x1B,0xF9,0x05,0xFE,0xFB,0x05,0xFF,0x2E,0x1A,0xF9,0x05,0xFE,0xFB,0x12,0xFF,0xA6,0x19,0xF9,0x01,0x10,0x11,0xFB,0x02,0x12,0xFB,0x02,0x13,0xFB,0x02,0x14,0xFB,0x02,0x15,0xFB,0x02,0x16,0xFB,0x07,0x17,0xFB,0x09,0x18,0xFB,0x05,0xFA,0xFA,0x19,0xFB,0x01,0xFA,0x06,0x1A,0xFB,0x04,0xF4,0xF2,0x01,0x1B,0xFB,0xFD,0x1C,0xFB,0x05,0x1D,0xF2,0x01,0x1E,0x1F,0x20,0x21,0xFB,0x01,0xFF,0xA0,0x19,0xF9,0x01,0xF2,0x01,0x22,0xFB,0x05,0x23,0xFB,0x06,0x24,0xFB,0x03,0x25,0xFB,0x05,0xF2,0x01,0x26,0xFB,0x07,0x27,0xFB,0x0C,0xFA,0xFD,0x28,0xFB,0x08,0xFA,0xF7,0x29,0xFB,0x08,0xFA,0xFE,0x2A,0xFB,0x04,0xFA,0x0B,0x2B,0xFB,0x04,0xFA,0x03,0x2C,0xFB,0x05,0xF4,0xF2,0x01,0xFF,0x81,0x19,0xF9,0x01,0xFB,0x01,0x6B,0xFB,0x02,0xFB,0x02,0x6C,0xFB,0x02,0x6D,0xFB,0x02,0x6D,0xFB,0x02,0x6D,0xFF,0x16,0x1B,0x00,0xF9,0x01,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0xFB,0x02,0x6C,0xFB,0x02,0x6D,0xFF,0x29,0x1B,0xF9,0x07,0xFE,0xFB,0x06,0x2D,0xFB,0x01,0x2E,0xFF,0xC4,0x19,0xF9,0x07,0xFE,0xFB,0x06,0x2D,0xFB,0x01,0x2E,0xFB,0x02,0x2F,0xFB,0xFF,0x30,0xFB,0x01,0x31,0xFB,0xFE,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0xFF,0xFF,0x75,0x19,0xF9,0x01,0xFB,0x01,0x35,0xFB,0x01,0xF2,0x01,0x36,0xFB,0x08,0x37,0xF2,0x01,0x38,0xFB,0x07,0x39,0xFB,0x03,0x3A,0xFB,0x01,0x3B,0x3C,0xFB,0x02,0x3D,0xFB,0xFF,0x3E,0x3F,0x40,0xFB,0xFF,0x41,0xFB,0xF2,0xFE,0xFF,0x95,0x19,0xF9,0x03,0xFA,0xFF,0x66,0xFB,0xFE,0xFA,0x06,0x67,0xFB,0xFE,0xFA,0x09,0x68,0xFB,0xFF,0xFA,0x0C,0x69,0xFB,0xFD,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFA,0xFF,0xFB,0xFE,0x66,0xFB,0xFD,0xFA,0x06,0x67,0xFB,0xFD,0xFA,0x09,0x68,0xFB,0xFE,0xFA,0x0C,0x69,0xFB,0xFD,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0xFF,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x01,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0xF7,0x06,0x1C,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x01,0x0F,0xFF,0x49,0x1D,0xFB,0xFF,0xFA,0xFD,0xFF,0xE4,0x1B,0xFB,0x01,0xFF,0xDB,0x1B,0x66,0xFB,0x02,0xFA,0x03,0x67,0xFB,0xFF,0xFA,0x04,0x68,0xFA,0x05,0x69,0xFB,0xFE,0xF8,0x01,0x06,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0x01,0xFA,0x09,0x68,0xFB,0x02,0xFA,0x0C,0x69,0xF8,0x02,0x0F,0xFF,0x49,0x1D,0xF9,0x03,0xFB,0x01,0xFA,0x03,0x66,0xFB,0x03,0xFA,0x06,0x67,0xFB,0x02,0xFA,0x09,0x68,0xFB,0x03,0xFA,0x0C,0x69,0xF8,0x03,0x0F,0xFF,0x49,0x1D,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0xF9,0x02,0x4E,0x4F,0x50,0xFF,0xA1,0x1C,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0xF9,0x02,0xFB,0x01,0x4E,0xFB,0x02,0x4F,0xFB,0x01,0x50,0xFF,0xA1,0x1C,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0xFB,0xFF,0x4D,0xF9,0x02,0xFB,0xFE,0x4E,0xFB,0xFF,0x4F,0xFB,0xFF,0x50,0xFF,0xA1,0x1C,0xF9,0x02,0x5B,0x5A,0x59,0x58,0x57,0x57,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x61,0x60,0x5F,0x5E,0x5D,0x5C,0x5B,0x5A,0x59,0x58,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x5F,0x5E,0x5D,0x5C,0xFF,0x25,0x1D,0xF9,0x06,0x5C,0x5D,0x5D,0x5C,0x5C,0x5B,0xFF,0xD8,0x1C,0x87,0x88,0x89,0x89,0x8A,0x8A,0x8A,0x8A,0x89,0x88,0x87,0xFB,0xF9,0xFF,0x25,0x1D,0xF9,0x01,0x94,0x91,0x90,0x8F,0x8E,0x8D,0xFB,0xFB,0xFA,0x3F,0xFC,0xF9,0x03,0x8C,0x8A,0x88,0x5B,0xF9,0x02,0xFF,0xA4,0x1C,0xF9,0x01,0x87,0x88,0x89,0x8A,0x8B,0x8C,0xFB,0x05,0xFA,0xC1,0xFD,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0xF9,0x05,0x95,0xF9,0x01,0x76,0x77,0xFB,0x01,0xFF,0xA0,0x19,0x51,0x52,0xF9,0x05,0x53,0xF9,0x01,0xF4,0xF2,0x00,0x54,0x55,0xFB,0x03,0xFF,0xA0,0x19,0xF9,0x03,0x51,0xFA,0x06,0x51,0xFA,0x09,0x51,0xFA,0x0C,0xFB,0x02,0xF8,0x00,0x0C,0xFF,0x49,0x1D,0xF9,0x04,0x6A,0xFF,0x4B,0x1D,0xF9,0x01,0x35,0xFB,0x02,0xF2,0x01,0x36,0xFB,0x07,0x37,0xF2,0x01,0x38,0xFB,0x02,0x31,0xFB,0xFE,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0xF9,0x00,0xF5,0x4F,0xFF,0x25,0x1D,0xF9,0x01,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0xFA,0xFC,0x4F,0xFA,0xFE,0x4F,0x4F,0xFA,0x02,0x4F,0xFA,0x04,0xFF,0x25,0x1D,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0xFA,0xFF,0x4D,0xFA,0xFD,0x4E,0xFA,0xFC,0x4F,0xFA,0xF6,0x4F,0xFA,0xF7,0x4F,0xFA,0xF8,0x4F,0xFA,0xF9,0x4F,0xFA,0xFA,0x4F,0xFA,0xFB,0x4F,0xFA,0xFC,0x4F,0xFA,0xFD,0x4F,0xFA,0xFE,0x4F,0xFA,0xFE,0x4F,0xFA,0xFF,0x4F,0xFA,0xFF,0x4F,0xFA,0xFF,0x4F,0x4F,0x4F,0x4F,0xFA,0x01,0x4F,0xFA,0x01,0x4F,0xFA,0x02,0x4F,0xFA,0x02,0x4F,0xFA,0x03,0x4F,0xFA,0x04,0x4F,0xFA,0x05,0x4F,0xFA,0x06,0x4F,0xF8,0x00,0x06,0xFF,0x49,0x1D,0xF9,0x03,0x50,0xFF,0xA1,0x1C,0xF9,0x05,0xFB,0xFC,0x32,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x05,0xFB,0x01,0xFA,0x03,0xF7,0x25,0x1E,0x66,0xFB,0x02,0xFA,0x06,0x67,0xFB,0xFF,0xFA,0x09,0x68,0xFA,0x0C,0x69,0xFB,0xFE,0xF8,0x00,0x0F,0xFF,0x49,0x1D,0x66,0xFB,0x02,0xFA,0x03,0x67,0xFB,0xFF,0xFA,0x04,0x68,0xFA,0x05,0x69,0xFB,0xFE,0xF8,0x00,0x06,0xFF,0x49,0x1D,0xF9,0x05,0xFB,0xFF,0xFA,0xFC,0x66,0xFB,0xFF,0xFA,0x03,0xFB,0xFD,0xFA,0x01,0xF4,0xFB,0x01,0xF2,0x01,0x6B,0xFB,0x02,0x6C,0xF2,0x01,0x6D,0xFF,0xB3,0x1F,0x79,0xFB,0x01,0x7A,0x7B,0xFB,0x02,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFC,0x56,0xF2,0x01,0xF4,0xFB,0xFC,0x74,0xFB,0xFE,0x75,0x76,0x77,0xFF,0xA0,0x19,0xFB,0xFB,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x03,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x02,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0xFB,0x01,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFF,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0x03,0x7E,0xFB,0xFE,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFF,0xFC,0x1E,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x04,0x7D,0xFB,0xFF,0x7F,0x80,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x03,0x7C,0xFB,0x02,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x7C,0xFB,0x02,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x7C,0xFB,0x01,0x81,0x82,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x02,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x7B,0xFB,0x01,0x83,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x7A,0xFB,0x01,0x84,0xFF,0xA0,0x19,0xF9,0x01,0x79,0xFB,0x01,0x84,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0x6D,0xFF,0xAF,0x1F,0xF9,0x05,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0x72,0x73,0x74,0xFB,0xFC,0x75,0x76,0x77,0xFF,0xA0,0x19,0xF9,0x01,0xF3,0x01,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE6,0xE7,0xE8,0xFF,0xDA,0x1F,0xF9,0x01,0xFB,0xFB,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0x85,0x85,0x86,0x86,0x86,0x30,0xFB,0x01,0x31,0xFB,0xFE,0xF9,0x05,0x32,0xF9,0x01,0x33,0x34,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0xFB,0xEA,0xEC,0xEE,0xF0,0x86,0xFB,0xFF,0xFF,0xA0,0x19,0xF9,0x01,0xFB,0x04,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xF2,0x03,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCD,0xCD,0xF3,0x01,0xCD,0xCD,0xC9,0xC6,0xFB,0xFC,0xFF,0xA0,0x19,0xF9,0x05,0xF4,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0xF9,0x01,0x6D,0xFF,0x36,0x20,0xF9,0x01,0xFA,0xFE,0xFB,0x01,0x6B,0xFB,0x02,0x6C,0x6D,0xFB,0x01,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0xFA,0x01,0x72,0xFA,0x01,0x73,0xFB,0xFE,0xFF,0x78,0x19,0xF9,0x05,0xF4,0xFA,0xFE,0xFB,0x01,0xFB,0x02,0x6C,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0x6D,0xFB,0x01,0x6E,0x6E,0x6E,0x6F,0xFB,0x02,0x70,0x71,0xFB,0x01,0xFA,0x01,0x72,0xFA,0x01,0x73,0x74,0xFB,0xFC,0x75,0x76,0x77,0xFF,0xA0,0x19,0xF9,0x05,0xF4,0xFA,0xFE,0xFB,0x03,0xB9,0xF6,0xB9,0xFF,0xA5,0x20,0xF9,0x05,0xFF,0xAE,0x20,0xF9,0x01,0xF6,0xB3,0xB4,0xB5,0xB6,0xFB,0x01,0xB7,0xFB,0xFC,0xB9,0xFF,0xBA,0x20,0xF9,0x01,0xF4,0xFB,0x04,0xB1,0xF6,0xB1,0xFF,0xC5,0x20,0xF9,0x01,0xB2,0xF6,0xB2,0xFF,0xCD,0x20,0xFF,0x5A,0x20,0xF8,0x00,0x00,0xF9,0x04,0xB9,0xFF,0xD9,0x20,0xF9,0x05,0xFB,0xFB,0xFA,0xFF,0xF2,0x01,0xD9,0xDA,0xDB,0xFB,0x01,0xDC,0xFB,0xFC,0xFA,0xFD,0xF2,0x01,0xDD,0xFB,0xFC,0xFA,0xFE,0xDE,0xFB,0xFE,0xFA,0xFD,0xDF,0xFB,0xFD,0xFA,0xF8,0xF2,0x04,0xF2,0x01,0xE0,0xFB,0xFF,0xFA,0xFF,0xE1,0xFB,0xFD,0xFA,0xFC,0xE2,0xFB,0xFF,0xFA,0xFB,0xF2,0x01,0xE3,0xFB,0xFE,0xFA,0xFF,0xE4,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0x00,0x00,0x00,0xF2,0x01,0xF1,0x00,0xFF,0x2E,0x21,0x36,0xFF,0x32,0x21,0x55,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x53,0x54,0x4C,0xFF,0x4B,0x21,0xFB,0x01,0x30,0xFB,0x02,0x31,0xFB,0x06,0x32,0xFB,0x01,0x33,0xFB,0xFF,0x34,0xFB,0x01,0x35,0xFB,0x01,0xFF,0x51,0x21,0xFB,0x01,0x37,0x38,0xFF,0x32,0x21,0x4D,0x4E,0x4F,0x50,0x51,0x52,0xFB,0x01,0x36,0x36,0x36,0x36,0x36,0x36,0x39,0x3A,0x3B,0x3C,0x3D,0xFB,0x02,0x3E,0xFB,0xFF,0x3F,0xFB,0xFD,0x40,0x41,0xFB,0xFF,0x42,0xFE,0xFB,0x10,0xFB,0x03,0xFF,0x54,0x21,0x0B,0xFF,0x95,0x21,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFE,0xFB,0x08,0x0B,0xFF,0x95,0x21,0xFE,0xFB,0x0B,0x0C,0xFB,0x01,0x0D,0xFB,0x01,0x0E,0xFB,0x03,0x0F,0xFB,0x01,0x10,0x11,0xFF,0xB8,0x21,0x13,0xFF,0xBC,0x21,0x14,0xFF,0xC0,0x21,0x15,0xFB,0x01,0x16,0x17,0x18,0xFB,0x01,0x19,0xFB,0xFD,0x1A,0xFB,0xFE,0x1B,0xFB,0xFC,0x1C,0xFB,0xFD,0x1D,0xFB,0xFE,0x1E,0xFB,0xFD,0x1F,0xFB,0xFF,0x20,0x21,0xFF,0xE2,0x21,0x25,0xFF,0xE6,0x21,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0xFE,0xFB,0x0C,0x0B,0xFF,0xF8,0x21,0x0B,0x0B,0xFE,0xFB,0x0D,0x2F,0x2E,0x2D,0x2C,0x2B,0x2A,0x29,0x28,0x27,0x26,0x25,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x23,0x23,0x24,0xFF,0x3C,0x22,0x01,0x12,0xFF,0x41,0x22,0xF9,0x01,0xBA,0xFB,0x05,0xBA,0xFB,0x03,0xBB,0xFB,0x04,0xFF,0x47,0x22,0xBA,0xFF,0x53,0x22,0xBC,0xFF,0x57,0x22,0xF9,0x00,0xBA,0xBA,0xBA,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xFE,0xFB,0x08,0xFF,0x47,0x22,0xBA,0xBA,0xBC,0xFF,0x70,0x22};

const word original_seqtbl_offsets[] = {
	0x0000, 0x1973, 0x19A0, 0x1A93, 0x1ACD, 0x1B39, 0x1B5A, 0x1BDB,
	0x1C54, 0x1CA1, 0x1D04, 0x1D25, 0x1D49, 0x1D4F, 0x1D68, 0x1DF6,
	0x1C84, 0x202B, 0x1C1C, 0x1C01, 0x205A, 0x1C38, 0x209C, 0x1D36,
	0x1C69, 0x1CD1, 0x1B04, 0x1B1A, 0x1D7D, 0x1F9E, 0x1F92, 0x1F82,
	0x1F72, 0x1F5D, 0x1F48, 0x1F33, 0x1F19, 0x1F13, 0x1EF7, 0x1EDA,
	0x1EBB, 0x1E9C, 0x1E7D, 0x1B53, 0x1E59, 0x1E06, 0x1E3B, 0x1DFC,
	0x1D9B, 0x1FB3, 0x1FA7, 0x20BE, 0x20D1, 0x20D4, 0x20C9, 0x19C4,
	0x1A13, 0x1A2E, 0x1A3C, 0x1A7C, 0x1A83, 0x1A63, 0x1A5E, 0x1A68,
	0x1A6E, 0x1A75, 0x1A5A, 0x19F9, 0x1CEC, 0x1A54, 0x20DD, 0x20AE,
	0x1E78, 0x1CDC, 0x19DC, 0x1A42, 0x1A07, 0x19A8, 0x2009, 0x1B1B,
	0x1A8B, 0x1B85, 0x1BA1, 0x1BBF, 0x196E, 0x20A9, 0x1A22, 0x19A6,
	0x19AC, 0x1B2D, 0x19C1, 0x1FCA, 0x1FDA, 0x1FFB, 0x2195, 0x2132,
	0x214F, 0x2166, 0x2199, 0x21A8, 0x216D, 0x226E, 0x2136, 0x21BC,
	0x1BFA, 0x2245, 0x2253, 0x225B, 0x21C4, 0x21C0, 0x21E6, 0x21EA,
	0x21FC, 0x2240, 0x2257
};

void check_seqtable_matches_original() {
	printf("Checking that the sequence table matches the original DOS version...\n");
	int different = 0;
	int i;
	const byte* seq = seqtbl;
	const byte* original_seq = original_seqtbl;
	for(i = 0; i < COUNT(original_seqtbl); ++i) {
		if (seq[i] != original_seq[i]) {
			different = 1;
			printf("Seqtbl difference at index %d (%#x; shifted offset %#x): value is %d, should be %d\n"
			        , i, i, i + SEQTBL_BASE, seqtbl[i], original_seqtbl[i]);
		}
	}
	for(i = 0; i < COUNT(original_seqtbl_offsets); ++i) {
		if (seqtbl_offsets[i] != original_seqtbl_offsets[i]) {
			different = 1;
			printf("Seqtbl offset difference at index %d: value is %#x, should be %#x\n"
			        , i, seqtbl_offsets[i], original_seqtbl_offsets[i]);
		}
	}
	if (!different) printf("All good, no differences found!\n");
}

#endif // CHECK_SEQTABLE_MATCHES_ORIGINAL
