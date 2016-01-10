#include <stdio.h>
#include "types.h"
#include "data.h"

void __pascal far redraw_screen(int drawing_different_room);

int copied_tiles=0;
int copied_modif=0;

void editor__handle_mouse_button(SDL_MouseButtonEvent e,int shift, int ctrl, int alt) {
				int col,row,tile;
				col=e.x/32;
				row=(e.y-4)/64;
				if (row<0 || row>2) return;
				tile=row*10+col;

				switch(e.button) {
				case SDL_BUTTON_LEFT:
					curr_room_tiles[tile]=copied_tiles;
					curr_room_modif[tile]=copied_modif;
					redraw_screen(1);
					break;
				case SDL_BUTTON_RIGHT:
					copied_tiles=curr_room_tiles[tile];
					copied_modif=curr_room_modif[tile];
					break;
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

