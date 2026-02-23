#ifndef INSPECTOR_CONTEXT_H
#define INSPECTOR_CONTEXT_H

#include <stddef.h>

typedef struct inspector_context_t inspector_context_t;
typedef struct inspector_struct_info_t inspector_struct_info_t;

inspector_context_t*    inspector_context_create(const inspector_struct_info_t* info, const char* prefix);
void                    inspector_context_destroy(inspector_context_t* ctx);
int                     inspector_context_update(inspector_context_t* ctx, const void* data);
const void*             inspector_context_get_data(const inspector_context_t* ctx);

#endif /* INSPECTOR_CONTEXT_H */
