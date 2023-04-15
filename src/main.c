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

#include "common.h"


#ifdef __amigaos4__
static const char version[] = "\0$VER: SDLPoP " SDLPOP_VERSION " (" __AMIGADATE__ ")";
static const char stack[] = "$STACK:200000";
#endif


int main(int argc, char *argv[])
{
	g_argc = argc;
	g_argv = argv;
	pop_main();
	return 0;
}

