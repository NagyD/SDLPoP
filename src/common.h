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

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
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

#define snprintf_check(dst, size, ...)	do {			\
		int __len;					\
		__len = snprintf(dst, size, __VA_ARGS__);	\
		if (__len < 0 || __len >= (int)size) {		\
			fprintf(stderr, "%s: buffer truncation detected!\n", __func__);\
			quit(2);				\
		}						\
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
