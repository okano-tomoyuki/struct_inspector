#include <stdlib.h>
#include <string.h>
#include "inspector_context.h"
#include "inspector.h"
#include "inspector_dsl.h"

struct inspector_context_t
{
    inspector_t*                    insp;
    const inspector_struct_info_t*  info;
    void*                           data;
    size_t                          size;
};

inspector_context_t *inspector_context_create(const inspector_struct_info_t *info, const char *prefix)
{
    if (!info)
    {
        return NULL;
    }

    inspector_context_t *ctx = (inspector_context_t *)malloc(sizeof(inspector_context_t));
    if (!ctx)
    {
        return NULL;
    }

    ctx->insp = inspector_create();
    if (!ctx->insp)
    {
        free(ctx);
        return NULL;
    }

    ctx->info = info;
    ctx->size = info->size;

    ctx->data = malloc(ctx->size);
    if (!ctx->data)
    {
        inspector_destroy(ctx->insp);
        free(ctx);
        return NULL;
    }

    if (prefix)
    {
        inspector_bind_struct(ctx->insp, ctx->info, ctx->data, prefix);
    }
    else
    {
        inspector_bind_struct(ctx->insp, ctx->info, ctx->data, "");
    }

    return ctx;
}

void inspector_context_destroy(inspector_context_t *ctx)
{
    if (!ctx)
    {
        return;
    }

    if (ctx->insp)
    {
        inspector_destroy(ctx->insp);
    }

    if (ctx->data)
    {
        free(ctx->data);
    }

    free(ctx);
}

int inspector_context_update(inspector_context_t *ctx, const void *data)
{
    if (!ctx || !data || !ctx->data)
    {
        return -1;
    }

    memcpy(ctx->data, data, ctx->size);

    return 0;
}

const void *inspector_context_get_data(const inspector_context_t *ctx)
{
    if (!ctx)
    {
        return NULL;
    }
    return ctx->data;
}

int inspector_context_contains(const inspector_context_t *ctx, const char *name)
{
    if (!ctx || !ctx->insp)
        return 0;
    return inspector_contains(ctx->insp, name);
}

const char *inspector_context_type(const inspector_context_t *ctx, const char *name)
{
    if (!ctx || !ctx->insp)
        return NULL;
    return inspector_type(ctx->insp, name);
}

void *inspector_context_get(inspector_context_t *ctx, const char *name)
{
    if (!ctx || !ctx->insp)
        return NULL;
    return inspector_get(ctx->insp, name);
}

int inspector_context_size(const inspector_context_t *ctx)
{
    if (!ctx || !ctx->insp)
        return 0;
    return inspector_size(ctx->insp);
}

const char *inspector_context_name_at(const inspector_context_t *ctx, int index)
{
    if (!ctx || !ctx->insp)
        return NULL;
    return inspector_name_at(ctx->insp, index);
}

const char *inspector_context_type_at(const inspector_context_t *ctx, int index)
{
    if (!ctx || !ctx->insp)
        return NULL;
    return inspector_type_at(ctx->insp, index);
}
