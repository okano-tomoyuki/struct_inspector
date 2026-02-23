/**
 * @file inspector_dsl.h
 * @brief Macros and metadata structures used by the compile-time DSL that
 *        generates struct introspection information.
 *
 * This header provides lightweight descriptions of fields and structs that
 * can be emitted via the REGISTER_... macros. The generated symbols are
 * intended to be referenced by code that binds an inspector to an object.
 */

#ifndef INSPECTOR_DSL_H
#define INSPECTOR_DSL_H

#include <stddef.h>

/** Forward declaration for struct metadata emitted by the DSL. */
typedef struct inspector_struct_info_t inspector_struct_info_t;
/** Forward declaration of the runtime inspector type. */
typedef struct inspector_t inspector_t;

/**
 * @brief Describes a single field within a struct.
 *
 * - `name`: member name as a string.
 * - `offset`: byte offset of the member within the struct.
 * - `size`: total byte size of the member (scalars or arrays).
 * - `type_name`: human-readable type name.
 * - `nested`: pointer to nested struct metadata (for struct members) or NULL.
 * - `dim_count`: number of array dimensions (0 for scalars).
 * - `dims`: pointer to an array of dimension sizes (NULL for scalars).
 */
typedef struct inspector_field_info_t {
    const char*                     name;
    size_t                          offset;
    size_t                          size;
    const char*                     type_name;
    const inspector_struct_info_t*  nested;
    size_t                          dim_count;
    const size_t*                   dims;
} inspector_field_info_t;

/**
 * @brief Top-level metadata describing a struct type.
 *
 * - `struct_name`: type name as a string.
 * - `size`: sizeof(type).
 * - `field_count`: number of entries in `fields`.
 * - `fields`: pointer to an array of field descriptors.
 */
struct inspector_struct_info_t {
    const char*                     struct_name;
    size_t                          size;
    size_t                          field_count;
    const inspector_field_info_t*   fields;
};

/**
 * @brief Bind generated struct metadata to an inspector and object instance.
 *
 * This helper registers named references into the provided `inspector_t` for
 * each field described by `info`, using `obj` as the target storage and the
 * given `prefix` for name qualification.
 */
void inspector_bind_struct(inspector_t *insp, const inspector_struct_info_t *info, void *obj, const char *prefix);

/* The DSL macros are enabled when the build defines ENABLE_INSPECTOR_BIND_DSL.
 * When enabled, the REGISTER_... macros emit `static const` metadata symbols
 * suitable for including in headers. */
#ifdef ENABLE_INSPECTOR_BIND_DSL

#include "inspector_config.h"

#define _INSPECTOR_CONCAT_IMPL(a, b) a ## b
#define _INSPECTOR_CONCAT(a, b) _INSPECTOR_CONCAT_IMPL(a, b)
#define _INSPECTOR_CONCAT_POSTFIX(T) _INSPECTOR_CONCAT(T, INSPECTOR_STRUCT_INFO_POSTFIX)

#define REGISTER_STRUCT_INFO_BEGIN(T) \
    static const inspector_field_info_t T##_fields[] = {

#define REGISTER_STRUCT_INFO_END(T) \
    }; \
    static const inspector_struct_info_t _INSPECTOR_CONCAT_POSTFIX(T) = { \
        #T, sizeof(T), \
        sizeof(T##_fields)/sizeof(inspector_field_info_t), \
        T##_fields \
    };

#define REGISTER_FIELD_SCALAR(T, member, type, nested_ptr) \
    { #member, offsetof(T, member), sizeof(type), #type, \
      nested_ptr, 0, NULL },

#define REGISTER_FIELD_ARRAY(T, member, type, nested_ptr, dim_count, ...) \
    { #member, offsetof(T, member), \
      sizeof(type) * REGISTER_FIELD_ARRAY_MUL(dim_count, __VA_ARGS__), \
      #type, nested_ptr, dim_count, \
      (const size_t[]){ __VA_ARGS__ } },

#define REGISTER_FIELD_ARRAY_MUL(dim_count, ...) REGISTER_FIELD_ARRAY_MUL_IMPL(dim_count, __VA_ARGS__)
#define REGISTER_FIELD_ARRAY_MUL_IMPL(dim_count, ...) REGISTER_FIELD_ARRAY_MUL_##dim_count(__VA_ARGS__)

#define REGISTER_FIELD_ARRAY_MUL_1(d1) (d1)
#define REGISTER_FIELD_ARRAY_MUL_2(d1,d2) ((d1)*(d2))
#define REGISTER_FIELD_ARRAY_MUL_3(d1,d2,d3) ((d1)*(d2)*(d3))
#define REGISTER_FIELD_ARRAY_MUL_4(d1,d2,d3,d4) ((d1)*(d2)*(d3)*(d4))
#define REGISTER_FIELD_ARRAY_MUL_5(d1,d2,d3,d4,d5) ((d1)*(d2)*(d3)*(d4)*(d5))
#define REGISTER_FIELD_ARRAY_MUL_6(d1,d2,d3,d4,d5,d6) ((d1)*(d2)*(d3)*(d4)*(d5)*(d6))
#define REGISTER_FIELD_ARRAY_MUL_7(d1,d2,d3,d4,d5,d6,d7) ((d1)*(d2)*(d3)*(d4)*(d5)*(d6)*(d7))
#define REGISTER_FIELD_ARRAY_MUL_8(d1,d2,d3,d4,d5,d6,d7,d8) ((d1)*(d2)*(d3)*(d4)*(d5)*(d6)*(d7)*(d8))
#define REGISTER_FIELD_ARRAY_MUL_9(d1,d2,d3,d4,d5,d6,d7,d8,d9) ((d1)*(d2)*(d3)*(d4)*(d5)*(d6)*(d7)*(d8)*(d9))
#define REGISTER_FIELD_ARRAY_MUL_10(d1,d2,d3,d4,d5,d6,d7,d8,d9,d10) ((d1)*(d2)*(d3)*(d4)*(d5)*(d6)*(d7)*(d8)*(d9)*(d10))
#define REGISTER_FIELD_ARRAY_MUL_11(d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11) ((d1)*(d2)*(d3)*(d4)*(d5)*(d6)*(d7)*(d8)*(d9)*(d10)*(d11))
#define REGISTER_FIELD_ARRAY_MUL_12(d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12) ((d1)*(d2)*(d3)*(d4)*(d5)*(d6)*(d7)*(d8)*(d9)*(d10)*(d11)*(d12))

#endif // ENABLE_INSPECTOR_BIND_DSL

#endif // INSPECTOR_DSL_H