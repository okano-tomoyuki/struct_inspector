#ifndef INSPECTOR_BIND_H
#define INSPECTOR_BIND_H

#include "inspector_dsl.h"
#include "struct_inspector.h"

typedef void (*inspect_callback_t)(
    const char* full_path,
    void*       value_ptr,
    size_t      size,
    const char* type_name,
    void*       user_data
);

void inspect_struct_with_path(
    const struct_info_t* info,
    void*                obj,
    const char*          parent_path,
    inspect_callback_t   cb,
    void*                user_data
);

void inspect_array_with_path(
    const struct_info_t* elem_info,
    char*                base,
    const char*          parent_path,
    inspect_callback_t   cb,
    void*                user_data,
    size_t               dim,
    size_t               dim_count,
    const size_t*        dims,
    size_t               elem_size
);

void inspector_bind_struct(
    inspector_t*         insp,
    const struct_info_t* info,
    void*                obj,
    const char*          prefix /* e.g. "a" or "" */
);

#endif /* INSPECTOR_BIND_H */