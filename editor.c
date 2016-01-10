#include <stdio.h>
#include "types.h"
#include "data.h"

void __pascal far redraw_screen(int drawing_different_room);

int copied_tiles=0;
int copied_modif=0;





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
	if (stack_top<stack_cursor) stack_top=stack_cursor;
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
	unsigned char before=offset[(char*)(&level)];
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
		copied_tiles=curr_room_tiles[tile];
		copied_modif=curr_room_modif[tile];
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && !ctrl) { //shift+left click: move kid
		editor__position(&Kid,col,row,loaded_room,x,6563);
	} else if (e.button==SDL_BUTTON_RIGHT && shift && !alt && !ctrl) { //shift+right click: move kid
		editor__position(&Guard,col,row,loaded_room,x,6569);
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

