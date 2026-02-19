#ifndef STRUCT_INSPECTOR_H
#define STRUCT_INSPECTOR_H

typedef struct inspector_t inspector_t;

inspector_t*    inspector_create(void);
void            inspector_destroy(inspector_t* obj);
void            inspector_add(inspector_t* obj, const char* name, const char* type, void* ref, int* indices, int indices_len);
int             inspector_contains(const inspector_t* obj, const char* name);
const char*     inspector_type(const inspector_t* obj, const char* name);
void*           inspector_get(const inspector_t* obj, const char* name);
int             inspector_size(const inspector_t* obj);
const char*     inspector_name_at(const inspector_t* obj, int index);
const char*     inspector_type_at(const inspector_t* obj, int index);

/* 可変長引数あり版（配列要素） */
#define ADD_INSPECTOR_INDEXED(obj, member, type, ...) \
    do { int indices[] = { __VA_ARGS__ }; inspector_add(obj, #member, #type, &(member), indices, (int)(sizeof(indices) / sizeof(int))); } while (0)

#define ADD_INSPECTOR(obj, member, type) \
    do { inspector_add(obj, #member, #type, &(member), NULL, 0); } while (0)

#endif //