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

#include "common.h"
#include "savestate.h"


void serialize_to_file(void* data, size_t data_size, FILE* fp) {
    fwrite(data, data_size, 1, fp);
}

void deserialize_from_file(void* data, size_t data_size, FILE* fp) {
    fread(data, data_size, 1, fp);
}

void serialize_to_membuf(void* data, size_t data_size, membuf_type* buf) {
    memcpy(buf->curr_ptr, data, data_size);
    buf->curr_ptr += data_size;
    buf->cnt += data_size;
}

void deserialize_from_membuf(void* data, size_t data_size, membuf_type* buf) {
    memcpy(data, buf->curr_ptr, data_size);
    buf->curr_ptr += data_size;
    buf->cnt += data_size;
}

void savelist_serialize(savelist_type* savelist, serialization_func_type serialize_func, void* stream) {
    int num_vars = savelist->num_vars;
    serialize_func(&num_vars, sizeof(num_vars), stream);
    int i;
    for (i = 0; i < num_vars; ++i) {
        savelist_var_type* var = &savelist->vars[i];
        byte var_name_len = var->name_len;
        word var_data_size = var->data_size;
        serialize_func(&var_name_len,  sizeof(var_name_len),  stream);
        serialize_func(var->name,      var_name_len,          stream);
        serialize_func(&var_data_size, sizeof(var_data_size), stream);
        serialize_func(var->data,      var_data_size,         stream);
    }
}

void savelistvar_deserialize(savelist_var_type* var_dest, void* data_buffer, serialization_func_type deserialize_func, void* stream) {
    byte name_len = 0;
    word data_size = 0;

    deserialize_func(&name_len, sizeof(name_len), stream);
    name_len = (byte) MIN(name_len, SAVELIST_MAX_VAR_NAME_LEN);
    var_dest->name_len = name_len;
    deserialize_func(var_dest->name, name_len, stream);

    deserialize_func(&data_size, sizeof(data_size), stream);
    data_size = (word) MIN(data_size, SAVELIST_MAX_VAR_SIZE);
    var_dest->data_size = data_size;

    deserialize_func(data_buffer, data_size, stream); // this retrieves the actual data of the variable
}

void savelist_deserialize(savelist_type *savelist, int num_vars_read, serialization_func_type deserialize_func, void* stream) {
    // Reserve enough memory as a buffer for the largest possible savelist variable
    byte* var_buffer = malloc(SAVELIST_MAX_VAR_SIZE);

    // Read savestate's variables
    int i;
    for (i = 0; i < num_vars_read; ++i) {
        savelist_var_type the_var = {0};
        savelistvar_deserialize(&the_var, var_buffer, deserialize_func, stream);

        // Match with the script's registered variables
        int curr_var_id;
        for (curr_var_id = 0; curr_var_id < savelist->num_vars; ++curr_var_id) {
            if (strncmp(the_var.name, savelist->vars[curr_var_id].name, SAVELIST_MAX_VAR_NAME_LEN) == 0) {
                goto found;
            }
        }
        fprintf(stderr, "Warning: Savestate contains unregistered variable \"%s\".\n", the_var.name);
        continue; // Matching script var not found, discard and read the next var in the savestate

        found:
        {
            // Matching script var found, try to replace that var's data with the data from the savestate
            savelist_var_type* found_var = &savelist->vars[curr_var_id];
            word savelist_var_data_size = found_var->data_size;

            if (savelist_var_data_size != the_var.data_size) {
                fprintf(stderr, "Warning: Restored savestate variable \"%s\" has an unexpected size "
                                "(%d bytes, expected %d bytes).\n",
                        found_var->name, the_var.data_size, savelist_var_data_size);
            }
            memset(found_var->data, 0, savelist_var_data_size);
            memcpy(found_var->data, var_buffer, MIN(the_var.data_size, savelist_var_data_size));
        }
    }
    free(var_buffer);
}

