/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2015  Dávid Nagy

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

#ifndef SAVESTATE_H
#define SAVESTATE_H

#define SAVELIST_MAX_VARS 256
#define SAVELIST_MAX_VAR_SIZE 65536
#define SAVELIST_MAX_VAR_NAME_LEN 64

#define SAVESTATE_OPTIONVARS_HEADER_BYTE 'O'
#define SCRIPT_SAVELIST_HEADER_BYTE 'S'

typedef struct savelist_var_type {
    byte name_len;
    char name[SAVELIST_MAX_VAR_NAME_LEN];
    word data_size;
    void* data;
} savelist_var_type;

typedef struct savelist_type {
    int num_vars;
    savelist_var_type vars[SAVELIST_MAX_VARS];
} savelist_type;


// Helper type, similar to the FILE struct, so that we can keep track of a 'current position' within a chunk of memory
typedef struct membuf_type {
    byte* curr_ptr;
    off_t cnt;
    byte* base;
} membuf_type;

typedef void (*serialization_func_type)(void* data, size_t data_size, void* stream);

void serialize_to_file(void* data, size_t data_size, FILE* fp);
void deserialize_from_file(void* data, size_t data_size, FILE* fp);
void serialize_to_membuf(void* data, size_t data_size, membuf_type* buf);
void deserialize_from_membuf(void* data, size_t data_size, membuf_type* buf);

void savelist_serialize(savelist_type* savelist, serialization_func_type serialize_func, void* stream);
void savelist_deserialize(savelist_type *savelist, int num_vars_read, serialization_func_type deserialize_func, void* stream);

#endif // SAVESTATE_H