#include <stdio.h>
#include "types.h"
#include "data.h"

void __pascal far redraw_screen(int drawing_different_room);

int copied_tiles=0;
int copied_modif=0;





/* do, undo, redo layer */

typedef enum {
	mark_middle=0,
	mark_start=1,
	mark_end=2,
	mark_all=3
}tUndoQueueMark;

#define editor__do(field,c,mark) editor__do_( ((long)(&(((level_type*)NULL)->field))) ,c,mark)
void editor__do_(long offset, unsigned char c, tUndoQueueMark mark) {
	offset[(char*)(&level)]=c;
} 


int __pascal far get_tile_div_mod_m7(int xpos);

/**
convert SDL x to Prince c:
SDL 0 == Prince 54
SDL 320 == Prince 198
x*144/320+54

*/

void editor__position(char_type* character,int col,int row,int room,int x,word seq) {
	Kid.curr_col=col;
	Kid.curr_row=row;
	Kid.room=room;
	Kid.x=x;
	Kid.y=55+63*row;
	Kid.curr_seq=seq;
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
		copied_tiles=curr_room_tiles[tile];
		copied_modif=curr_room_modif[tile];
	} else if (e.button==SDL_BUTTON_LEFT && shift && !alt && !ctrl) { //shift+left click: move kid
		editor__position(&Kid,col,row,loaded_room,x,6563);
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

