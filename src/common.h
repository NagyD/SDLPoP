/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2019  Dávid Nagy

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

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef _MSC_VER // unistd.h does not exist in the Windows SDK.
#include <unistd.h>
#else
#ifndef _UNISTD_H
#define _UNISTD_H    1
#define F_OK    0       /* Test for existence.  */
#define access _access
#endif
#endif

// S_ISREG and S_ISDIR may not be defined under MSVC
#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#include "config.h"
#include "types.h"
#include "proto.h"
#include "data.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif

#ifdef __cplusplus
}
#endif

#endif
