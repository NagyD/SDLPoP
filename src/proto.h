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

// SEG000.C
void pop_main(void);
void init_game_main(void);
void start_game(void);
int process_key(void);
void play_frame(void);
void draw_game_frame(void);
void anim_tile_modif(void);
void load_sound_names(void);
void load_sounds(int min_sound,int max_sound);
void load_opt_sounds(int first,int last);
void load_lev_spr(int level);
void load_level(void);
void reset_level_unused_fields(bool loading_clean_level);
int play_kid_frame(void);
void play_guard_frame(void);
void check_the_end(void);
void check_fall_flo(void);
void read_joyst_control(void);
void draw_kid_hp(short curr_hp,short max_hp);
void draw_guard_hp(short curr_hp,short max_hp);
void add_life(void);
void set_health_life(void);
void draw_hp(void);
void do_delta_hp(void);
void play_sound(int sound_id);
void play_next_sound(void);
void check_sword_vs_sword(void);
void load_chtab_from_file(int chtab_id,int resource,const char* filename,int palette_bits);
void free_all_chtabs_from(int first);
void load_more_opt_graf(const char* filename);
int do_paused(void);
void read_keyb_control(void);
void copy_screen_rect(const rect_type* source_rect_ptr);
void toggle_upside(void);
void feather_fall(void);
int parse_grmode(void);
void gen_palace_wall_colors(void);
void show_title(void);
void transition_ltr(void);
void release_title_images(void);
void draw_full_image(enum full_image_id id);
void load_kid_sprite(void);
void save_game(void);
short load_game(void);
void clear_screen_and_sounds(void);
void parse_cmdline_sound(void);
void free_optional_sounds(void);
void free_all_sounds(void);
void load_all_sounds(void);
void free_optsnd_chtab(void);
void load_title_images(int bgcolor);
void show_copyprot(int where);
void show_loading(void);
void show_quotes(void);
void show_splash(void);
#ifdef USE_QUICKSAVE
void check_quick_op(void);
void restore_room_after_quick_load(void);
#endif // USE_QUICKSAVE

// SEG001.C
int proc_cutscene_frame(int wait_frames);
void play_both_seq(void);
void draw_proom_drects(void);
void play_kid_seq(void);
void play_opp_seq(void);
void draw_princess_room_bg(void);
void seqtbl_offset_shad_char(int seq_index);
void seqtbl_offset_kid_char(int seq_index);
void init_mouse_cu8(void);
void init_mouse_go(void);
void princess_crouching(void);
void princess_stand(void);
void init_princess_x156(void);
void princess_lying(void);
void init_princess_right(void);
void init_ending_princess(void);
void init_mouse_1(void);
void init_princess(void);
void init_vizier(void);
void init_ending_kid(void);
void cutscene_8(void);
void cutscene_9(void);
void end_sequence_anim(void);
void time_expired(void);
void cutscene_12(void);
void cutscene_4(void);
void cutscene_2_6(void);
void pv_scene(void);
void set_hourglass_state(int state);
int hourglass_frame(void);
void princess_room_torch(void);
void draw_hourglass(void);
void reset_cutscene(void);
void do_flash(short color);
void delay_ticks(Uint32 ticks);
void remove_flash(void);
void end_sequence(void);
void expired(void);
void load_intro(int which_imgs,void (*func)(void),int free_sounds);
void draw_star(int which_star,int mark_dirty);
void show_hof(void);
void hof_write(void);
void hof_read(void);
void show_hof_text(rect_type* rect,int x_align,int y_align, const char* text);
int fade_in_1(void);
int fade_out_1(void);

// SEG002.C
void do_init_shad(const byte* source,int seq_index);
void get_guard_hp(void);
void check_shadow(void);
void enter_guard(void);
void check_guard_fallout(void);
void leave_guard(void);
void follow_guard(void);
void exit_room(void);
int goto_other_room(short direction);
short leave_room(void);
void Jaffar_exit(void);
void level3_set_chkp(void);
void sword_disappears(void);
void meet_Jaffar(void);
void play_mirr_mus(void);
void move_0_nothing(void);
void move_1_forward(void);
void move_2_backward(void);
void move_3_up(void);
void move_4_down(void);
void move_up_back(void);
void move_down_back(void);
void move_down_forw(void);
void move_6_shift(void);
void move_7(void);
void autocontrol_opponent(void);
void autocontrol_mouse(void);
void autocontrol_shadow(void);
void autocontrol_skeleton(void);
void autocontrol_Jaffar(void);
void autocontrol_kid(void);
void autocontrol_guard(void);
void autocontrol_guard_inactive(void);
void autocontrol_guard_active(void);
void autocontrol_guard_kid_far(void);
void guard_follows_kid_down(void);
void autocontrol_guard_kid_in_sight(short distance);
void autocontrol_guard_kid_armed(short distance);
void guard_advance(void);
void guard_block(void);
void guard_strike(void);
void hurt_by_sword(void);
void check_sword_hurt(void);
void check_sword_hurting(void);
void check_hurting(void);
void check_skel(void);
void do_auto_moves(const auto_move_type* moves_ptr);
void autocontrol_shadow_level4(void);
void autocontrol_shadow_level5(void);
void autocontrol_shadow_level6(void);
void autocontrol_shadow_level12(void);

// SEG003.C
void init_game(int level);
void play_level(int level_number);
void do_startpos(void);
void set_start_pos(void);
void find_start_level_door(void);
void draw_level_first(void);
void redraw_screen(int drawing_different_room);
int play_level_2(void);
void redraw_at_char(void);
void redraw_at_char2(void);
void check_knock(void);
void timers(void);
void check_mirror(void);
void jump_through_mirror(void);
void check_mirror_image(void);
void bump_into_opponent(void);
void pos_guards(void);
void check_can_guard_see_kid(void);
byte get_tile_at_kid(int xpos);
void do_mouse(void);
int flash_if_hurt(void);
void remove_flash_if_hurt(void);

// SEG004.C
void check_collisions(void);
void move_coll_to_prev(void);
void get_row_collision_data(short row,sbyte* row_coll_room_ptr,byte* row_coll_flags_ptr);
int get_left_wall_xpos(int room,int column,int row);
int get_right_wall_xpos(int room,int column,int row);
void check_bumped(void);
void check_bumped_look_left(void);
void check_bumped_look_right(void);
int is_obstacle_at_col(int tile_col);
int is_obstacle(void);
int xpos_in_drawn_room(int xpos);
void bumped(sbyte delta_x,sbyte direction);
void bumped_fall(void);
void bumped_floor(sbyte direction);
void bumped_sound(void);
void clear_coll_rooms(void);
int can_bump_into_gate(void);
int get_edge_distance(void);
void check_chomped_kid(void);
void chomped(void);
void check_gate_push(void);
void check_guard_bumped(void);
void check_chomped_guard(void);
int check_chomped_here(void);
int dist_from_wall_forward(byte tiletype);
int dist_from_wall_behind(byte tiletype);

// SEG005.C
void seqtbl_offset_char(short seq_index);
void seqtbl_offset_opp(int seq_index);
void do_fall(void);
void land(void);
void spiked(void);
void control(void);
void control_crouched(void);
void control_standing(void);
void up_pressed(void);
void down_pressed(void);
void go_up_leveldoor(void);
void control_turning(void);
void crouch(void);
void back_pressed(void);
void forward_pressed(void);
void control_running(void);
void safe_step(void);
int check_get_item(void);
void get_item(void);
void control_startrun(void);
void control_jumpup(void);
void standing_jump(void);
void check_jump_up(void);
void jump_up_or_grab(void);
void grab_up_no_floor_behind(void);
void jump_up(void);
void control_hanging(void);
void can_climb_up(void);
void hang_fall(void);
void grab_up_with_floor_behind(void);
void run_jump(void);
void back_with_sword(void);
void forward_with_sword(void);
void draw_sword(void);
void control_with_sword(void);
void swordfight(void);
void sword_strike(void);
void parry(void);
#ifdef USE_TELEPORTS
void teleport();
#endif

// SEG006.C
int get_tile(int room,int col,int row);
int find_room_of_tile(void);
int get_tilepos(int tile_col,int tile_row);
int get_tilepos_nominus(int tile_col,int tile_row);
void load_fram_det_col(void);
void determine_col(void);
void load_frame(void);
short dx_weight(void);
int char_dx_forward(int delta_x);
int obj_dx_forward(int delta_x);
void play_seq(void);
int get_tile_div_mod_m7(int xpos);
int get_tile_div_mod(int xpos);
int sub_70B6(int ypos);
int y_to_row_mod4(int ypos);
void loadkid(void);
void savekid(void);
void loadshad(void);
void saveshad(void);
void loadkid_and_opp(void);
void savekid_and_opp(void);
void loadshad_and_opp(void);
void saveshad_and_opp(void);
void reset_obj_clip(void);
void x_to_xh_and_xl(int xpos, sbyte* xh_addr, sbyte* xl_addr);
void fall_accel(void);
void fall_speed(void);
void check_action(void);
int tile_is_floor(int tiletype);
void check_spiked(void);
int take_hp(int count);
int get_tile_at_char(void);
void set_char_collision(void);
void check_on_floor(void);
void start_fall(void);
void check_grab(void);
int can_grab_front_above(void);
void in_wall(void);
int get_tile_infrontof_char(void);
int get_tile_infrontof2_char(void);
int get_tile_behind_char(void);
int distance_to_edge_weight(void);
int distance_to_edge(int xpos);
void fell_out(void);
void play_kid(void);
void control_kid(void);
void do_demo(void);
void play_guard(void);
void user_control(void);
void flip_control_x(void);
int release_arrows(void);
void save_ctrl_1(void);
void rest_ctrl_1(void);
void clear_saved_ctrl(void);
void read_user_control(void);
int can_grab(void);
int wall_type(byte tiletype);
int get_tile_above_char(void);
int get_tile_behind_above_char(void);
int get_tile_front_above_char(void);
int back_delta_x(int delta_x);
void do_pickup(int obj_type);
void check_press(void);
void check_spike_below(void);
void clip_char(void);
void stuck_lower(void);
void set_objtile_at_char(void);
void proc_get_object(void);
int is_dead(void);
void play_death_music(void);
void on_guard_killed(void);
void clear_char(void);
void save_obj(void);
void load_obj(void);
void draw_hurt_splash(void);
void check_killed_shadow(void);
void add_sword_to_objtable(void);
void control_guard_inactive(void);
int char_opp_dist(void);
void inc_curr_row(void);
#ifdef USE_JUMP_GRAB
bool check_grab_run_jump(void);
#endif

// SEG007.C
void process_trobs(void);
void animate_tile(void);
short is_trob_in_drawn_room(void);
void set_redraw_anim_right(void);
void set_redraw_anim_curr(void);
void redraw_at_trob(void);
void redraw_21h(void);
void redraw_11h(void);
void redraw_20h(void);
void draw_trob(void);
void redraw_tile_height(void);
short get_trob_pos_in_drawn_room(void);
short get_trob_right_pos_in_drawn_room(void);
short get_trob_right_above_pos_in_drawn_room(void);
void animate_torch(void);
void animate_potion(void);
void animate_sword(void);
void animate_chomper(void);
void animate_spike(void);
void animate_door(void);
void gate_stop(void);
void animate_leveldoor(void);
short bubble_next_frame(short curr);
short get_torch_frame(short curr);
void set_redraw_anim(short tilepos,byte frames);
void set_redraw2(short tilepos,byte frames);
void set_redraw_floor_overlay(short tilepos, byte frames);
void set_redraw_full(short tilepos,byte frames);
void set_redraw_fore(short tilepos,byte frames);
void set_wipe(short tilepos,byte frames);
void start_anim_torch(short room,short tilepos);
void start_anim_potion(short room,short tilepos);
void start_anim_sword(short room,short tilepos);
void start_anim_chomper(short room,short tilepos, byte modifier);
void start_anim_spike(short room,short tilepos);
short trigger_gate(short room,short tilepos,short button_type);
short trigger_1(short target_type,short room,short tilepos,short button_type);
void do_trigger_list(short index,short button_type);
void add_trob(byte room,byte tilepos,sbyte type);
short find_trob(void);
void clear_tile_wipes(void);
short get_doorlink_timer(short index);
short set_doorlink_timer(short index,byte value);
short get_doorlink_tile(short index);
short get_doorlink_next(short index);
short get_doorlink_room(short index);
void trigger_button(int playsound,int button_type,int modifier);
void died_on_button(void);
void animate_button(void);
void start_level_door(short room,short tilepos);
void animate_empty(void);
void animate_loose(void);
void loose_shake(int arg_0);
int remove_loose(int room, int tilepos);
void make_loose_fall(byte modifier);
void start_chompers(void);
int next_chomper_timing(byte timing);
void loose_make_shake(void);
void do_knock(int room,int tile_row);
void add_mob(void);
short get_curr_tile(short tilepos);
void do_mobs(void);
void move_mob(void);
void move_loose(void);
void loose_land(void);
void loose_fall(void);
void redraw_at_cur_mob(void);
void mob_down_a_row(void);
void draw_mobs(void);
void draw_mob(void);
void add_mob_to_objtable(int ypos);
void sub_9A8E(void);
int is_spike_harmful(void);
void check_loose_fall_on_kid(void);
void fell_on_your_head(void);
void play_door_sound_if_visible(int sound_id);

// SEG008.C
void redraw_room(void);
void load_room_links(void);
void draw_room(void);
void draw_tile(void);
void draw_tile_aboveroom(void);
void redraw_needed(short tilepos);
void redraw_needed_above(int column);
int get_tile_to_draw(int room, int column, int row, byte* ptr_tile, byte* ptr_modifier, byte tile_room0);
void load_curr_and_left_tile(void);
void load_leftroom(void);
void load_rowbelow(void);
void draw_tile_floorright(void);
int can_see_bottomleft(void);
void draw_tile_topright(void);
void draw_tile_anim_topright(void);
void draw_tile_right(void);
int get_spike_frame(byte modifier);
void draw_tile_anim_right(void);
void draw_tile_bottom(word arg_0);
void draw_loose(int arg_0);
void draw_tile_base(void);
void draw_tile_anim(void);
void draw_tile_fore(void);
int get_loose_frame(byte modifier);
int add_backtable(short chtab_id, int id, sbyte xh, sbyte xl, int ybottom, int blit, byte peel);
int add_foretable(short chtab_id, int id, sbyte xh, sbyte xl, int ybottom, int blit, byte peel);
int add_midtable(short chtab_id, int id, sbyte xh, sbyte xl, int ybottom, int blit, byte peel);
void add_peel(int left,int right,int top,int height);
void add_wipetable(sbyte layer,short left,short bottom,sbyte height,short width,sbyte color);
void draw_table(int which_table);
void draw_wipes(int which);
void draw_back_fore(int which_table,int index);
void draw_mid(int index);
void draw_image(image_type *image,image_type *mask,int xpos,int ypos,int blit);
void draw_wipe(int index);
void calc_gate_pos(void);
void draw_gate_back(void);
void draw_gate_fore(void);
void alter_mods_allrm(void);
void load_alter_mod(int tile_ix);
void draw_moving(void);
void redraw_needed_tiles(void);
void draw_tile_wipe(byte height);
void draw_tables(void);
void restore_peels(void);
void add_drect(rect_type *source);
void draw_leveldoor(void);
void get_room_address(int room);
void draw_floor_overlay(void);
void draw_other_overlay(void);
void draw_tile2(void);
void draw_objtable_items_at_tile(byte tilepos);
void sort_curr_objs(void);
int compare_curr_objs(int index1,int index2);
void draw_objtable_item(int index);
int load_obj_from_objtable(int index);
void draw_people(void);
void draw_kid(void);
void draw_guard(void);
void add_kid_to_objtable(void);
void add_guard_to_objtable(void);
void add_objtable(byte obj_type);
void mark_obj_tile_redraw(int index);
void load_frame_to_obj(void);
void show_time(void);
void show_level(void);
short calc_screen_x_coord(short logical_x);
void free_peels(void);
void display_text_bottom(const char *text);
void erase_bottom_text(int arg_0);
void wall_pattern(int which_part,int which_table);
void draw_left_mark (word arg3, word arg2, word arg1);
void draw_right_mark (word arg2, word arg1);
image_type* get_image(short chtab_id, int id);

// SEG009.C
void sdlperror(const char* header);
bool file_exists(const char* filename);
#define locate_file(filename) locate_file_(filename, alloca(POP_MAX_PATH), POP_MAX_PATH)
const char* locate_file_(const char* filename, char* path_buffer, int buffer_size);

#ifdef _WIN32

FILE* fopen_UTF8(const char* filename, const char* mode);
#define fopen fopen_UTF8

int chdir_UTF8(const char* path);
#define chdir chdir_UTF8

int mkdir_UTF8(const char* path);
#define mkdir mkdir_UTF8

int access_UTF8(const char* filename_UTF8, int mode);
#ifdef access
#undef access
#endif
#define access access_UTF8

int stat_UTF8(const char *filename_UTF8, struct stat *_Stat);
// We define a function-like macro, because `stat` is also the name of the type, and we don't want to redefine that.
#define stat(filename_UTF8, _Stat) stat_UTF8(filename_UTF8, _Stat)

#endif //_WIN32

directory_listing_type* create_directory_listing_and_find_first_file(const char* directory, const char* extension);
char* get_current_filename_from_directory_listing(directory_listing_type* data);
bool find_next_file(directory_listing_type* data);
void close_directory_listing(directory_listing_type* data);
int read_key(void);
void clear_kbd_buf(void);
word prandom(word max);
int round_xpos_to_byte(int xpos,int round_direction);
void show_dialog(const char* text);
void quit(int exit_code);
void restore_stuff(void);
int key_test_quit(void);
const char* check_param(const char *param);
int pop_wait(int timer_index,int time);
dat_type *open_dat(const char* file, int optional);
void set_loaded_palette(dat_pal_type *palette_ptr);
chtab_type* load_sprites_from_file(int resource,int palette_bits, int quit_on_error);
void free_chtab(chtab_type *chtab_ptr);
image_type* decode_image(image_data_type* image_data, dat_pal_type* palette);
image_type*load_image(int index, dat_pal_type* palette);
void draw_image_transp(image_type* image,image_type* mask,int xpos,int ypos);
int set_joy_mode(void);
surface_type *make_offscreen_buffer(const rect_type* rect);
void free_surface(surface_type *surface);
void free_peel(peel_type *peel_ptr);
void set_hc_pal(void);
void flip_not_ega(byte* memory,int height,int stride);
void flip_screen(surface_type* surface);
void fade_in_2(surface_type* source_surface,int which_rows);
void fade_out_2(int rows);
void draw_image_transp_vga(image_type* image,int xpos,int ypos);
int get_line_width(const char *text,int length);
int draw_text_character(byte character);
void draw_rect(const rect_type *rect,int color);
void draw_rect_with_alpha(const rect_type* rect, byte color, byte alpha);
void draw_rect_contours(const rect_type* rect, byte color);
surface_type *rect_sthg(surface_type* surface,const rect_type* rect);
rect_type *shrink2_rect(rect_type* target_rect,const rect_type* source_rect,int delta_x,int delta_y);
void set_curr_pos(int xpos,int ypos);
void restore_peel(peel_type* peel_ptr);
peel_type* read_peel_from_screen(const rect_type *rect);
void show_text(const rect_type* rect_ptr,int x_align,int y_align,const char* text);
int intersect_rect(rect_type* output,const rect_type* input1,const rect_type* input2);
rect_type * union_rect(rect_type* output,const rect_type* input1,const rect_type* input2);
void stop_sounds(void);
void init_digi(void);
void play_sound_from_buffer(sound_buffer_type* buffer);
void turn_music_on_off(byte new_state);
void turn_sound_on_off(byte new_state);
int check_sound_playing(void);
void apply_aspect_ratio(void);
void window_resized(void);
void set_gr_mode(byte grmode);
SDL_Surface* get_final_surface(void);
void update_screen(void);
void set_pal_arr(int start,int count,const rgb_type* array);
void set_pal(int index,int red,int green,int blue);
int add_palette_bits(byte n_colors);
void process_palette(void* target,dat_pal_type* source);
int find_first_pal_row(int which_rows_mask);
int get_text_color(int cga_color,int low_half,int high_half_mask);
void close_dat(dat_type* pointer);
void *load_from_opendats_alloc(int resource, const char* extension, data_location* out_result, int* out_size);
int load_from_opendats_to_area(int resource,void* area,int length, const char* extension);
void rect_to_sdlrect(const rect_type* rect, SDL_Rect* sdlrect);
void method_1_blit_rect(surface_type* target_surface,surface_type* source_surface,const rect_type* target_rect, const rect_type* source_rect,int blit);
image_type* method_3_blit_mono(image_type* image,int xpos,int ypos,int blitter,byte color);
const rect_type* method_5_rect(const rect_type* rect,int blit,byte color);
void draw_rect_with_alpha(const rect_type* rect, byte color, byte alpha);
image_type* method_6_blit_img_to_scr(image_type* image,int xpos,int ypos,int blit);
void reset_timer(int timer_index);
double get_ticks_per_sec(int timer_index);
void set_timer_length(int timer_index, int length);
void start_timer(int timer_index, int length);
int do_wait(int timer_index);
void init_timer(int frequency);
void set_clip_rect(const rect_type* rect);
void reset_clip_rect(void);
void set_bg_attr(int vga_pal_index,int hc_pal_index);
rect_type* offset4_rect_add(rect_type* dest, const rect_type* source,int d_left,int d_top,int d_right,int d_bottom);
int input_str(const rect_type* rect,char *buffer,int max_length,const char *initial,int has_initial,int arg_4,int color,int bgcolor);
rect_type* offset2_rect(rect_type* dest, const rect_type* source,int delta_x,int delta_y);
void show_text_with_color(const rect_type* rect_ptr,int x_align,int y_align, const char *text,int color);
void do_simple_wait(int timer_index);
void process_events(void);
void idle(void);
void init_copyprot_dialog(void);
dialog_type* make_dialog_info(dialog_settings_type* settings, rect_type* dialog_rect,
                                            rect_type* text_rect, peel_type* dialog_peel);
void calc_dialog_peel_rect(dialog_type* dialog);
void read_dialog_peel(dialog_type* dialog);
void draw_dialog_frame(dialog_type* dialog);
void add_dialog_rect(dialog_type* dialog);
void dialog_method_2_frame(dialog_type* dialog);
#ifdef USE_FADE
void fade_in_2(surface_type* source_surface,int which_rows);
palette_fade_type *make_pal_buffer_fadein(surface_type* source_surface,int which_rows,int wait_time);
void pal_restore_free_fadein(palette_fade_type* palette_buffer);
int fade_in_frame(palette_fade_type* palette_buffer);
void fade_out_2(int rows);
palette_fade_type* make_pal_buffer_fadeout(int which_rows,int wait_time);
void pal_restore_free_fadeout(palette_fade_type* palette_buffer);
int fade_out_frame(palette_fade_type* palette_buffer);
void read_palette_256(rgb_type* target);
void set_pal_256(rgb_type* source);
#endif
void set_chtab_palette(chtab_type* chtab, byte* colors, int n_colors);
int has_timer_stopped(int index);
sound_buffer_type* load_sound(int index);
void free_sound(sound_buffer_type* buffer);

// SEQTABLE.C
void apply_seqtbl_patches(void);
#ifdef CHECK_SEQTABLE_MATCHES_ORIGINAL
void check_seqtable_matches_original();
#endif

// OPTIONS.C
void turn_fixes_and_enhancements_on_off(byte new_state);
void turn_custom_options_on_off(byte new_state);
void set_options_to_default(void);
void load_global_options(void);
void check_mod_param(void);
void load_mod_options(void);
int process_rw_write(SDL_RWops* rw, void* data, size_t data_size);
int process_rw_read(SDL_RWops* rw, void* data, size_t data_size);
void load_dos_exe_modifications(const char* folder_name);

// REPLAY.C
#ifdef USE_REPLAY
void start_with_replay_file(const char *filename);
void init_record_replay(void);
void replay_restore_level(void);
int restore_savestate_from_buffer(void);
void start_recording(void);
void add_replay_move(void);
void stop_recording(void);
void start_replay(void);
void end_replay(void);
void do_replay_move(void);
int save_recorded_replay_dialog(void);
int save_recorded_replay(const char* full_filename);
void replay_cycle(void);
int load_replay(void);
void key_press_while_recording(int* key_ptr);
void key_press_while_replaying(int* key_ptr);
#endif

// lighting.c
#ifdef USE_LIGHTING
void init_lighting(void);
void redraw_lighting(void);
void update_lighting(const rect_type* source_rect_ptr);
#endif

// screenshot.c
#ifdef USE_SCREENSHOT
void save_screenshot(void);
void auto_screenshot(void);
bool want_auto_screenshot(void);
void init_screenshot(void);
void save_level_screenshot(bool want_extras);
#endif

// menu.c
#ifdef USE_MENU
void init_menu(void);
void menu_scroll(int y);
void draw_menu(void);
void clear_menu_controls(void);
void process_additional_menu_input(void);
int key_test_paused_menu(int key);
void load_ingame_settings(void);
void menu_was_closed(void);
#endif

// midi.c
void stop_midi(void);
void init_midi(void);
void midi_callback(void *userdata, Uint8 *stream, int len);
void play_midi_sound(sound_buffer_type* buffer);
