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
    const size_t*         dims;  /* ポインタに変更：動的に多次元配列に対応可能 */
} field_info_t;

struct struct_info_t {
    const char*      struct_name;
    size_t           size;
    size_t           field_count;
    const field_info_t* fields;
};

void inspector_bind_struct(inspector_t *insp, const struct_info_t *info, void *obj, const char *prefix);

#ifdef ENABLE_STRUCT_INSEPCTOR_BIND_DSL

#define REGISTER_STRUCT_INFO_BEGIN(T) \
    static const field_info_t T##_fields[] = {

#define REGISTER_STRUCT_INFO_END(T) \
    }; \
    const struct_info_t T##_info = { \
        #T, sizeof(T), \
        sizeof(T##_fields)/sizeof(field_info_t), \
        T##_fields \
    };

#define REGISTER_FIELD_SCALAR(T, member, type, nested_ptr) \
    { #member, offsetof(T, member), sizeof(type), #type, \
      nested_ptr, 0, NULL },  /* dims は NULL（スカラー型） */

#define REGISTER_FIELD_ARRAY(T, member, type, nested_ptr, dim_count, ...) \
    { #member, offsetof(T, member), \
      sizeof(type) * REGISTER_FIELD_ARRAY_MUL(dim_count, __VA_ARGS__), \
      #type, nested_ptr, dim_count, \
      (const size_t[]){ __VA_ARGS__ } },  /* 複合リテラル（C99）：多次元対応 */

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

/* 
 * 12次元以上の多次元配列に対応したい場合は、同じパターンで追加定義してください。
 * 例：13次元の場合、REGISTER_FIELD_ARRAY_MUL_13 を追加
 */

#endif

#endif /* STRUCT_META_H */