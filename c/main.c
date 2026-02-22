#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "struct_inspector.h"
#include "inspector_dsl.h"
#include "binder.h"

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
    int value;
} a_t;

STRUCT_INFO_BEGIN(d_t) 
    FIELD_ARRAY(d_t, value, char, NULL, 1, 32) 
STRUCT_INFO_END(d_t) 

STRUCT_INFO_BEGIN(c_t) 
    FIELD_ARRAY(c_t, d, d_t, &d_t_info, 1, D_SIZE) 
    FIELD_SCALAR(c_t, value, int, NULL) 
STRUCT_INFO_END(c_t)

STRUCT_INFO_BEGIN(b_t) 
    FIELD_ARRAY(b_t, c, c_t, &c_t_info, 1, C_SIZE) 
    FIELD_SCALAR(b_t, value, float, NULL) 
STRUCT_INFO_END(b_t)

STRUCT_INFO_BEGIN(a_t) 
    FIELD_ARRAY(a_t, b, b_t, &b_t_info, 1, B_SIZE) 
    FIELD_SCALAR(a_t, value, int, NULL) 
STRUCT_INFO_END(a_t)

int main(void)
{
    inspector_t* inspector = inspector_create();
    a_t a = {0};

    inspector_bind_struct(inspector, &a_t_info, &a, "a");

    for (int i = 0; i < B_SIZE; i++)
    {
        a.b[i].value = i * 0.001;
        for (int j = 0; j < C_SIZE; j++)
        {
            a.b[i].c[j].value = i + j * 0.001;
            for (int k = 0; k < D_SIZE; k++)
            {
                sprintf(a.b[i].c[j].d[k].value, "%d-%d-%d", i, j, k);
            }
        }
    }

    for (int i = 0; i < inspector_size(inspector); i++)
    {
        const char* name = inspector_name_at(inspector, i);
        const char* type = inspector_type_at(inspector, i);

        if (strcmp(type, "int") == 0)
        {
            printf("%s found, type=%s, value=%d\n", name, type, *(int*)inspector_get(inspector, name));
        }
        else if (strcmp(type, "float") == 0)
        {
            printf("%s found, type=%s, value=%f\n", name, type, *(float*)inspector_get(inspector, name));
        }
        else if (strcmp(type, "double") == 0)
        {
            printf("%s found, type=%s, value=%f\n", name, type, *(double*)inspector_get(inspector, name));
        }
    }

    char name[STR_SIZE];
    for (int i = 0; i < B_SIZE; i++)
    {
        for (int j = 0; j < C_SIZE; j++)
        {
            for (int k = 0; k < D_SIZE; k++)
            {
                sprintf(name, "a.b[%d].c[%d].d[%d].value", i, j, k);
                printf("%s = %s\n", name, (const char*)inspector_get(inspector, name));
            }
        }
    }

    inspector_destroy(inspector);
    return 0;
}