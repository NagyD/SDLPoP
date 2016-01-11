#include "common.h"
#ifdef USE_EDITOR

void __pascal far redraw_screen(int drawing_different_room);

int copied_tiles=0;
int copied_modif=0;


int edition_level=-1;
level_type edited;
void editor__load_level() {
	dat_type* dathandle;
	dathandle = open_dat("LEVELS.DAT", 0);
	load_from_opendats_to_area(current_level + 2000, &edited, sizeof(edited), "bin");
	close_dat(dathandle);
	edition_level=current_level;
}





/* do, undo, redo layer */
#define true 1
#define false 0

int stack_top=0;
int stack_size=0;
int stack_cursor=0;
long* stack_pointer=NULL;

void stack_push(long data) {
	if (!stack_size) {
		stack_size=100;
		stack_pointer=malloc(sizeof(long)*stack_size);
	}
	stack_pointer[stack_cursor]=data;
	stack_cursor++;
	/*if (stack_top<stack_cursor)*/ stack_top=stack_cursor;
	if (stack_size<=stack_top) {
		stack_size*=2;
		stack_pointer=realloc(stack_pointer,sizeof(long)*stack_size);
	}
}
int stack_pop(long* data) {
	if (!stack_cursor) return false;
	*data=stack_pointer[--stack_cursor];
	return true;
}
int stack_unpop(long* data) {
	if (stack_cursor==stack_top) return false;
	*data=stack_pointer[stack_cursor++];
	return true;
}
#undef true
#undef false



typedef enum {
	mark_middle=0,
	mark_start=1,
	mark_end=2,
	mark_all=3
}tUndoQueueMark;

#define editor__do(field,c,mark) editor__do_( ((long)(&(((level_type*)NULL)->field))) ,c,mark)
void editor__do_(long offset, unsigned char c, tUndoQueueMark mark) {
	unsigned char before;

	if (edition_level!=current_level)
		editor__load_level();

	before=offset[(char*)(&edited)];
	offset[(char*)(&edited)]=c;
	offset[(char*)(&level)]=c;
	stack_push(offset<<18|mark<<16|before<<8|c);
} 
void editor__undo() {
	long aux,offset;
	unsigned char before;
	tUndoQueueMark mark;
	while (stack_pop(&aux)) {
		//after=   aux     & 0xff;
		before= (aux>>8) & 0xff;
		mark=   (aux>>16)& mark_all;
		offset= (aux>>18);
		offset[(char*)(&level)]=before;
		offset[(char*)(&edited)]=before;
		if (mark & mark_start) break;
	}
}
void editor__redo() {
	long aux,offset;
	unsigned char after;
	tUndoQueueMark mark;
	while (stack_unpop(&aux)) {
		after=   aux     & 0xff;
		//before= (aux>>8) & 0xff;
		mark=   (aux>>16)& mark_all;
		offset= (aux>>18);
		offset[(char*)(&level)]=after;
		offset[(char*)(&edited)]=after;
		if (mark & mark_end) break;
	}
}


int __pascal far get_tile_div_mod_m7(int xpos);

/**
convert SDL x to Prince c:
SDL 0 == Prince 54
SDL 320 == Prince 198
x*144/320+54

*/

void editor__position(char_type* character,int col,int row,int room,int x,word seq) {
	character->curr_col=col;
	character->curr_row=row;
	character->room=room;
	character->x=x;
	character->y=55+63*row;
	character->curr_seq=seq;
}

/*
Debug char position
printf("Guard | %d-%d %d,%d |\n", 
	Guard.curr_col,Guard.curr_row,Guard.x,Guard.y
);
*/

void save_resource(const char* file, int res, const void* data, int size, const char* ext) {
	char aux[255];
	FILE* fp;
	snprintf(aux,255,"data/%s/res%d.%s",file,res,ext);
	fp=fopen(aux,"wb");
	if (fp) {
		fwrite(data,size,1,fp);
		fclose(fp);
	} else {
		printf("error opening '%s'\n",aux);
	}
}

void save_level() {
	save_resource("LEVELS.DAT",current_level + 2000, &edited, sizeof(edited), "bin");
}

void editor__set_guard(tile,x) {
	printf("tile %d\n",level.guards_tile[loaded_room-1]);
	printf("c %d\n",level.guards_color[loaded_room-1]);
	printf("x %d\n",level.guards_x[loaded_room-1]);
	printf("dir %d\n",level.guards_dir[loaded_room-1]);
	printf("skill %d\n",level.guards_skill[loaded_room-1]);
	if (level.guards_tile[loaded_room-1]>=30) {
		editor__do(guards_tile[loaded_room-1],tile,mark_start);
		editor__do(guards_color[loaded_room-1],0,mark_middle);
		editor__do(guards_x[loaded_room-1],x,mark_middle);
		editor__do(guards_dir[loaded_room-1],0,mark_middle);
		editor__do(guards_skill[loaded_room-1],0,mark_end);
	} else {
		editor__do(guards_tile[loaded_room-1],tile,mark_all);
	}
}
void editor__remove_guard() {
	editor__do(guards_tile[loaded_room-1],30,mark_all);
}

void editor__handle_mouse_button(SDL_MouseButtonEvent e,int shift, int ctrl, int alt) {
	int col,row,tile,x;
	col=e.x/32;
	row=(e.y-4)/64;
	x=e.x*140/320+62;
	if (row<0 || row>2) return;
	tile=row*10+col;

	if (e.button==SDL_BUTTON_LEFT && !shift && !alt && !ctrl) { //left click: edit tile
		editor__do(fg[(loaded_room-1)*30+tile],copied_tiles,mark_start);
		editor__do(bg[(loaded_room-1)*30+tile],copied_modif,mark_end);
		redraw_screen(1);
	} else if (e.button==SDL_BUTTON_RIGHT && !shift && !alt && !ctrl) { //right click: copy tile
		copied_tiles=edited.fg[(loaded_room-1)*30+tile];
		copied_modif=edited.bg[(loaded_room-1)*30+tile];
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && !ctrl) { //shift+left click: move kid
		editor__position(&Kid,col,row,loaded_room,x,6563);
	} else if (e.button==SDL_BUTTON_RIGHT && shift && !alt && !ctrl) { //shift+right click: move move/put guard
		editor__set_guard(tile,x);
		editor__position(&Guard,col,row,loaded_room,x,6569);
		redraw_screen(1);
	}
			
				/*printf("hola mundo %d %d %d %d %c%c%c\n", 
					e.state,
					e.button,
					tile,
					current_level,
					shift?'s':' ',
					ctrl?'c':' ',
					alt?'a':' '
	);*/
}
void editor__process_key(int key,const char** answer_text, word* need_show_text) {

	switch (key) {
	case SDL_SCANCODE_Z | WITH_CTRL: // ctrl-z
		editor__undo();
		redraw_screen(1);
		break;
	case SDL_SCANCODE_Z | WITH_CTRL | WITH_ALT: // ctrl-alt-z
		editor__redo();
		redraw_screen(1);
		break;
	case SDL_SCANCODE_K | WITH_SHIFT: // shift-k
		editor__remove_guard();
		//TODO: finish this: deactivate Guard object
		redraw_screen(1);
		break;
	//TODO: [] for guard lives. {} for guard skill. <> for guard color. g mirror guard
	case SDL_SCANCODE_C | WITH_CTRL: // ctrl-c
		if (edition_level!=-1) {
			*answer_text="LEVEL SAVED";
			*need_show_text=1;
			save_level();
		} else {
			*answer_text="Nothing to save";
			*need_show_text=1;
		}
		break;
	}
}


#endif
