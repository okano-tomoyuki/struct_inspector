#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector.h"
#include "inspector_context.h"

#define ENABLE_INSEPCTOR_BIND_DSL
#include "inspector_dsl.h"

#define STR_SIZE (32)
#define B_SIZE   (8)
#define C_SIZE   (4)
#define D_SIZE   (2)

typedef struct d_t
{
    char value[STR_SIZE];
} d_t;

typedef struct c_t
{
    d_t d[D_SIZE];
    double value;
} c_t;

typedef struct b_t
{
    c_t c[C_SIZE];
    float value;
} b_t;

typedef struct a_t
{
    b_t b[B_SIZE];
    c_t c[B_SIZE][C_SIZE];
    d_t d[B_SIZE][C_SIZE][D_SIZE];
    int value;
} a_t;

REGISTER_STRUCT_INFO_BEGIN(d_t) 
    REGISTER_FIELD_ARRAY(d_t, value, char, NULL, 1, STR_SIZE) 
REGISTER_STRUCT_INFO_END(d_t) 

REGISTER_STRUCT_INFO_BEGIN(c_t) 
    REGISTER_FIELD_ARRAY(c_t, d, d_t, &d_t_info, 1, D_SIZE) 
    REGISTER_FIELD_SCALAR(c_t, value, double, NULL) 
REGISTER_STRUCT_INFO_END(c_t)

REGISTER_STRUCT_INFO_BEGIN(b_t) 
    REGISTER_FIELD_ARRAY(b_t, c, c_t, &c_t_info, 1, C_SIZE) 
    REGISTER_FIELD_SCALAR(b_t, value, float, NULL) 
REGISTER_STRUCT_INFO_END(b_t)

REGISTER_STRUCT_INFO_BEGIN(a_t) 
    REGISTER_FIELD_ARRAY(a_t, b, b_t, &b_t_info, 1, B_SIZE) 
    REGISTER_FIELD_ARRAY(a_t, c, c_t, &c_t_info, 2, B_SIZE, C_SIZE) 
    REGISTER_FIELD_ARRAY(a_t, d, d_t, &d_t_info, 3, B_SIZE, C_SIZE, D_SIZE) 
    REGISTER_FIELD_SCALAR(a_t, value, int, NULL) 
REGISTER_STRUCT_INFO_END(a_t)

int main(void)
{
    /* inspector_context を作成（メモリ確保 + バインド） */
    inspector_context_t* ctx = inspector_context_create(&a_t_info, "a");
    if (!ctx)
    {
        fprintf(stderr, "Failed to create inspector context\n");
        return 1;
    }

    /* 初期データをコンテキストにセット */
    a_t a = {0};
    if (inspector_context_update(ctx, &a) != 0)
    {
        fprintf(stderr, "Failed to update context\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* コンテキストが所有するポインタを取得 */
    a_t* a_ptr = (a_t*)inspector_context_get_data(ctx);
    if (!a_ptr)
    {
        fprintf(stderr, "Failed to get data\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* データを設定 */
    for (int i = 0; i < B_SIZE; i++)
    {
        a_ptr->b[i].value = i * 0.001;
        for (int j = 0; j < C_SIZE; j++)
        {
            a_ptr->b[i].c[j].value = i + j * 0.001;
            for (int k = 0; k < D_SIZE; k++)
            {
                sprintf(a_ptr->b[i].c[j].d[k].value, "%d-%d-%d", i, j, k);
            }
        }
    }
    a_ptr->value = 42;

    /* データアクセスの例 */
    printf("=== Data Access Examples ===\n");
    printf("a.value = %d\n", a_ptr->value);
    printf("a.b[0].value = %f\n", a_ptr->b[0].value);
    printf("a.b[1].c[0].value = %f\n", a_ptr->b[1].c[0].value);
    printf("a.b[2].c[1].d[0].value = %s\n", a_ptr->b[2].c[1].d[0].value);
    printf("a.b[7].c[3].value = %f\n", a_ptr->b[7].c[3].value);

    /* データをコンテキストに反映（変更後） */
    if (inspector_context_update(ctx, a_ptr) != 0)
    {
        fprintf(stderr, "Failed to update context\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* 別のデータで更新（動的更新の例） */
    a_t a2 = {0};
    a2.value = 100;
    for (int i = 0; i < B_SIZE; i++)
    {
        a2.b[i].value = i * 0.01;
    }
    if (inspector_context_update(ctx, &a2) != 0)
    {
        fprintf(stderr, "Failed to update context with new data\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* 更新後のデータを取得 */
    a_ptr = (a_t*)inspector_context_get_data(ctx);
    printf("\n=== After Update ===\n");
    printf("a.value = %d\n", a_ptr->value);
    printf("a.b[3].value = %f\n", a_ptr->b[3].value);

    /* クリーンアップ */
    inspector_context_destroy(ctx);
    return 0;
}