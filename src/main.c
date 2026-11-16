/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2025  Dávid Nagy

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

#ifdef __PSP__
#include <psppower.h>
#endif

#ifdef __SWITCH__
// On Nintendo Switch the entry point is main() in switch_main.c, which calls sdlpop_main().
int sdlpop_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	#ifdef __PSP__
	scePowerSetClockFrequency(333,333,166);
	#endif
	g_argc = argc;
	g_argv = argv;
	pop_main();
	return 0;
}

