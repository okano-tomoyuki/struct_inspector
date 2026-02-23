#include <stdlib.h>
#include <string.h>
#include "inspector_context.h"
#include "inspector.h"
#include "inspector_dsl.h"

/**
 * inspector_context_t の実装構造体
 */
struct inspector_context_t
{
    inspector_t*                    inspector;      /* 内部の軽量インスペクター */
    const inspector_struct_info_t*  info;           /* メタ情報への参照 */
    void*                           data;           /* 保有するデータ（malloc確保） */
    size_t                          data_size;      /* データサイズ */
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

    ctx->inspector = inspector_create();
    if (!ctx->inspector)
    {
        free(ctx);
        return NULL;
    }

    ctx->info = info;
    ctx->data_size = info->size;

    /* この時点でデータメモリを確保（アドレスは以降固定） */
    ctx->data = malloc(ctx->data_size);
    if (!ctx->data)
    {
        inspector_destroy(ctx->inspector);
        free(ctx);
        return NULL;
    }

    /* メモリ確保後、内部の inspector_t にバインド（プレフィックス指定） */
    if (prefix)
    {
        inspector_bind_struct(ctx->inspector, ctx->info, ctx->data, prefix);
    }
    else
    {
        inspector_bind_struct(ctx->inspector, ctx->info, ctx->data, "");
    }

    return ctx;
}

void inspector_context_destroy(inspector_context_t *ctx)
{
    if (!ctx)
    {
        return;
    }

    if (ctx->inspector)
    {
        inspector_destroy(ctx->inspector);
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

    /* 既存メモリの内容を新しいデータで上書き */
    memcpy(ctx->data, data, ctx->data_size);

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
