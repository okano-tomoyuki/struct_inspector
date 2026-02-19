#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "struct_inspector.h"

typedef struct inspector_value_t 
{
    char  name[64];
    char  type[32];
    void* ref;
    struct inspector_value_t* next;
} inspector_value_t;

struct inspector_t
{
    inspector_value_t* root;
};

static void replace_indicies_string(char* str, int* indices, int indices_len)
{
    if (indices == NULL || indices_len == 0)
        return;

    char buf[64];
    char tmp[64];
    strcpy(buf, str);

    char* src = buf;
    char* dst = tmp;
    int   idx_pos = 0;

    while (*src && idx_pos < indices_len)
    {
        if (*src == '[')
        {
            // コピー '['
            *dst++ = *src++;
            // 元の [i] の中身をスキップ
            while (*src && *src != ']')
                src++;
            if (*src == ']')
                src++; // ']' もスキップ

            // インデックスを書き込む
            int n = snprintf(dst, (int)(sizeof(tmp) - (dst - tmp)), "%d]", indices[idx_pos++]);
            dst += n;
        }
        else
        {
            *dst++ = *src++;
        }
    }

    // 残りをコピー
    while (*src)
        *dst++ = *src++;
    *dst = '\0';

    strcpy(str, tmp);
}

inspector_t* inspector_create(void)
{
    inspector_t* ret = (inspector_t*)malloc(sizeof(inspector_t));
    if (!ret) return NULL;
    ret->root = NULL;
    return ret;
}

void inspector_destroy(inspector_t* obj)
{
    if (!obj) return;
    inspector_value_t* node = obj->root;
    while (node)
    {
        inspector_value_t* next = node->next;
        free(node);
        node = next;
    }
    free(obj);
}

void inspector_add(inspector_t* obj, const char* name, const char* type, void* ref, int* indices, int indices_len)
{
    char path[64];
    strcpy(path, name);
    replace_indicies_string(path, indices, indices_len);

    inspector_value_t* last_node = NULL;
    for (inspector_value_t* node = obj->root; node != NULL; node = node->next)
    {
        last_node = node;

        if (strcmp(node->name, path) == 0 && strcmp(node->type, type) == 0)
        {
            node->ref = ref;
            return;
        }
    }

    inspector_value_t* new_node = (inspector_value_t*)malloc(sizeof(inspector_value_t));
    if (!new_node) return;

    strcpy(new_node->name, path);
    strcpy(new_node->type, type);
    new_node->ref = ref;
    new_node->next = NULL;

    if (last_node)
        last_node->next = new_node;
    else
        obj->root = new_node;
}

int inspector_contains(const inspector_t* obj, const char* name)
{
    for (inspector_value_t* node = obj->root; node != NULL; node = node->next)
    {
        if (strcmp(node->name, name) == 0)
            return 1;
    }
    return 0;
}

const char* inspector_type(const inspector_t* obj, const char* name)
{
    for (inspector_value_t* node = obj->root; node != NULL; node = node->next)
    {
        if (strcmp(node->name, name) == 0)
            return node->type;
    }
    return NULL;
}

void* inspector_get(const inspector_t* obj, const char* name)
{
    for (inspector_value_t* node = obj->root; node != NULL; node = node->next)
    {
        if (strcmp(node->name, name) == 0)
            return node->ref;
    }
    return NULL;
}

int inspector_size(const inspector_t* obj)
{
    int len = -1;
    for (inspector_value_t* node = obj->root; node != NULL; node = node->next)
    {
        len++;
    }
    return len;    
}

const char* inspector_name_at(const inspector_t* obj, int index)
{
    int len = -1;
    for (inspector_value_t* node = obj->root; node != NULL; node = node->next)
    {
        len++;
        if (index == len)
        {
            return node->name;
        }
    }
    return NULL;
}

const char* inspector_type_at(const inspector_t* obj, int index)
{
    int len = -1;
    for (inspector_value_t* node = obj->root; node != NULL; node = node->next)
    {
        len++;
        if (index == len)
        {
            return node->type;
        }
    }
    return NULL;
}