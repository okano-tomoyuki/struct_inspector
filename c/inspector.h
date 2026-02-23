#ifndef INSPECTOR_H
#define INSPECTOR_H

typedef struct inspector_t inspector_t;

inspector_t*    inspector_create(void);
void            inspector_destroy(inspector_t* obj);
void            inspector_add(inspector_t* obj, const char* name, const char* type_name, void* ref);
int             inspector_contains(const inspector_t* obj, const char* name);
const char*     inspector_type(const inspector_t* obj, const char* name);
void*           inspector_get(const inspector_t* obj, const char* name);
int             inspector_size(const inspector_t* obj);
const char*     inspector_name_at(const inspector_t* obj, int index);
const char*     inspector_type_at(const inspector_t* obj, int index);

#endif // INSPECTOR_H