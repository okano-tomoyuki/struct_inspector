#ifndef STRUCT_META_H
#define STRUCT_META_H

#include <stddef.h>

typedef struct struct_info_t struct_info_t;
typedef struct inspector_t inspector_t;

typedef struct field_info_t {
    const char*           name;
    size_t                offset;
    size_t                size;
    const char*           type_name;
    const struct_info_t*  nested;
    size_t                dim_count;
    size_t                dims[4];
} field_info_t;

struct struct_info_t {
    const char*      struct_name;
    size_t           size;
    size_t           field_count;
    const field_info_t* fields;
};

void inspector_bind_struct(inspector_t *insp, const struct_info_t *info, void *obj, const char *prefix);

#ifdef ENABLE_STRUCT_INSEPCTOR_BIND_DSL

#define RESISTER_STRUCT_INFO_BEGIN(T) \
    static const field_info_t T##_fields[] = {

#define RESISTER_STRUCT_INFO_END(T) \
    }; \
    const struct_info_t T##_info = { \
        #T, sizeof(T), \
        sizeof(T##_fields)/sizeof(field_info_t), \
        T##_fields \
    };

#define RESISTER_FIELD_SCALAR(T, member, type, nested_ptr) \
    { #member, offsetof(T, member), sizeof(type), #type, \
      nested_ptr, 0, {0} },

#define RESISTER_FIELD_ARRAY(T, member, type, nested_ptr, dim_count, ...) \
    { #member, offsetof(T, member), sizeof(type) * RESISTER_FIELD_ARRAY_MUL(dim_count, __VA_ARGS__), \
      #type, nested_ptr, dim_count, { __VA_ARGS__ } },

#define RESISTER_FIELD_ARRAY_MUL(dim_count, ...) RESISTER_FIELD_ARRAY_MUL_IMPL(dim_count, __VA_ARGS__)
#define RESISTER_FIELD_ARRAY_MUL_IMPL(dim_count, ...) RESISTER_FIELD_ARRAY_MUL_##dim_count(__VA_ARGS__)

#define RESISTER_FIELD_ARRAY_MUL_1(d1) (d1)
#define RESISTER_FIELD_ARRAY_MUL_2(d1,d2) ((d1)*(d2))
#define RESISTER_FIELD_ARRAY_MUL_3(d1,d2,d3) ((d1)*(d2)*(d3))
#define RESISTER_FIELD_ARRAY_MUL_4(d1,d2,d3,d4) ((d1)*(d2)*(d3)*(d4))

#endif

#endif /* STRUCT_META_H */