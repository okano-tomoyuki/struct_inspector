#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "struct_inspector.h"

#define B_SIZE (8)
#define C_SIZE (4)
#define D_SIZE (2)

struct D
{
    char value;
};

struct C
{
    struct D d[D_SIZE];
    double value;
};

struct B
{
    struct C c[C_SIZE];
    float value;
};

struct A
{
    struct B b[B_SIZE];
    int value;
};

int main(void)
{
    inspector_t* inspector = inspector_create();
    struct A a = {0};

    ADD_INSPECTOR_INDEXED(inspector, a.value, int);

    for (int i = 0; i < B_SIZE; i++)
    {
        ADD_INSPECTOR_INDEXED(inspector, a.b[i].value, float, i);
        for (int j = 0; j < C_SIZE; j++)
        {
            ADD_INSPECTOR_INDEXED(inspector, a.b[i].c[j].value, double, i, j);
            for (int k = 0; k < D_SIZE; k++)
            {
                ADD_INSPECTOR_INDEXED(inspector, a.b[i].c[j].d[k].value, char, i, j, k);
            }
        }
    }

    for (int i = 0; i < inspector_size(inspector); i++)
    {
        const char* name = inspector_name_at(inspector, i);
        const char* type = inspector_type_at(inspector, i);

        if (strcmp(type, "char") == 0)
        {
            printf("%s found, type=%s, value=%d\n", name, type, *(char*)inspector_get(inspector, name));
        }
        else if (strcmp(type, "int") == 0)
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

    inspector_destroy(inspector);
    return 0;
}