#include <stdio.h>
#include <string.h>
#include "binder.h"

static void inspector_register_cb(
    const char *full_path,
    void *value_ptr,
    size_t size,
    const char *type_name,
    void *user_data)
{
    (void)size;
    inspector_t *insp = (inspector_t *)user_data;
    inspector_add(insp, full_path, type_name, value_ptr);
}

void inspect_struct_with_path(
    const struct_info_t *info,
    void *obj,
    const char *parent_path,
    inspect_callback_t cb,
    void *user_data)
{
    char path[256];

    for (size_t i = 0; i < info->field_count; i++)
    {
        const field_info_t *f = &info->fields[i];
        void *ptr = (char *)obj + f->offset;

        if (parent_path[0] == '\0')
        {
            snprintf(path, sizeof(path), "%s", f->name);
        }
        else
        {
            snprintf(path, sizeof(path), "%s.%s", parent_path, f->name);
        }

        if (f->nested)
        {
            if (f->dim_count == 0)
            {
                inspect_struct_with_path(f->nested, ptr, path, cb, user_data);
            }
            else
            {
                inspect_array_with_path(
                    f->nested,
                    (char *)ptr,
                    path,
                    cb,
                    user_data,
                    0,
                    f->dim_count,
                    f->dims,
                    f->nested->size
                );
            }
        }
        else
        {
            cb(path, ptr, f->size, f->type_name, user_data);
        }
    }
}

void inspect_array_with_path(
    const struct_info_t *elem_info,
    char *base,
    const char *parent_path,
    inspect_callback_t cb,
    void *user_data,
    size_t dim,
    size_t dim_count,
    const size_t *dims,
    size_t elem_size)
{
    char path[256];

    if (dim == dim_count)
    {
        inspect_struct_with_path(elem_info, base, parent_path, cb, user_data);
        return;
    }

    size_t stride = elem_size;
    for (size_t i = dim + 1; i < dim_count; i++)
        stride *= dims[i];

    for (size_t i = 0; i < dims[dim]; i++)
    {
        snprintf(path, sizeof(path), "%s[%zu]", parent_path, i);
        inspect_array_with_path(
            elem_info,
            base + i * stride,
            path,
            cb,
            user_data,
            dim + 1,
            dim_count,
            dims,
            elem_size
        );
    }
}

void inspector_bind_struct(
    inspector_t *insp,
    const struct_info_t *info,
    void *obj,
    const char *prefix)
{
    if (prefix && prefix[0] != '\0')
    {
        inspect_struct_with_path(info, obj, prefix, inspector_register_cb, insp);
    }
    else
    {
        inspect_struct_with_path(info, obj, "", inspector_register_cb, insp);
    }
}
