/*!
 * @file serialization_example.c
 * @brief Generic auto-serialization using inspector_context API.
 *
 * Demonstrates using inspector_context_size(), inspector_context_name_at(),
 * inspector_context_type_at(), and inspector_context_get() to build a
 * completely generic serializer that works on ANY registered struct.
 *
 * Real-world scenario: Auto-generate JSON/XML from struct metadata without
 * writing per-type serialization code.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Data structures ===== */

typedef struct
{
    int major;
    int minor;
    int patch;
} version_t;

REGISTER_STRUCT_INFO_BEGIN(version_t)
REGISTER_FIELD_SCALAR(version_t, major, int, NULL)
REGISTER_FIELD_SCALAR(version_t, minor, int, NULL)
REGISTER_FIELD_SCALAR(version_t, patch, int, NULL)
REGISTER_STRUCT_INFO_END(version_t)

typedef struct
{
    char name[64];
    version_t version;
    float cpu_usage;
    int memory_mb;
    int connections;
} service_status_t;

REGISTER_STRUCT_INFO_BEGIN(service_status_t)
REGISTER_FIELD_ARRAY(service_status_t, name, char, NULL, 1, 64)
REGISTER_FIELD_SCALAR(service_status_t, version, version_t, &version_t_info)
REGISTER_FIELD_SCALAR(service_status_t, cpu_usage, float, NULL)
REGISTER_FIELD_SCALAR(service_status_t, memory_mb, int, NULL)
REGISTER_FIELD_SCALAR(service_status_t, connections, int, NULL)
REGISTER_STRUCT_INFO_END(service_status_t)

/* ===== Generic serialization using inspector_context API ===== */

/**
 * Serialize a value based on its type name (works ANY type).
 */
static void serialize_value(const void *ptr, const char *type_name)
{
    if (!ptr)
    {
        printf("null");
        return;
    }

    if (strcmp(type_name, "int") == 0)
    {
        printf("%d", *(const int *)ptr);
    }
    else if (strcmp(type_name, "float") == 0)
    {
        printf("%.2f", *(const float *)ptr);
    }
    else if (strcmp(type_name, "double") == 0)
    {
        printf("%.6f", *(const double *)ptr);
    }
    else if (strcmp(type_name, "char") == 0)
    {
        printf("\"%s\"", (const char *)ptr);
    }
    else
    {
        printf("\"<unsupported: %s>\"", type_name);
    }
}

/**
 * Generic JSON-like serialization using inspector_context API.
 * Works on ANY struct without knowing its structure in advance.
 */
static void serialize_context_json(inspector_context_t *ctx, int indent)
{
    printf("{\n");

    int field_count = inspector_context_size(ctx);

    for (int i = 0; i < field_count; i++)
    {
        /* Use inspector_context API to discover field metadata */
        const char *field_name = inspector_context_name_at(ctx, i);
        const char *field_type = inspector_context_type_at(ctx, i);

        if (!field_name || !field_type)
            continue;

        /* Extract the base field name (remove prefix) */
        const char *display_name = field_name;
        if (strchr(field_name, '.'))
        {
            display_name = strchr(field_name, '.') + 1;
        }

        printf("%*s\"%s\": ", indent + 2, "", display_name);

        /* Get the actual field value pointer using inspector_context API */
        void *field_ptr = inspector_context_get(ctx, field_name);
        if (field_ptr)
        {
            serialize_value(field_ptr, field_type);
        }
        else
        {
            printf("null");
        }

        /* Add comma separator */
        if (i < field_count - 1)
        {
            printf(",\n");
        }
        else
        {
            printf("\n");
        }
    }

    printf("%*s}\n", indent, "");
}

/* ===== Main ===== */

int main(void)
{
    printf("=== Generic Serialization with inspector_context API ===\n\n");

    printf("KEY INSIGHT:\n");
    printf("The serializer knows NOTHING about service_status_t structure.\n");
    printf("It only uses inspector_context API to discover fields at runtime.\n\n");

    /* Create context for service status */
    inspector_context_t *ctx = inspector_context_create(
        &service_status_t_info,
        "service");

    const void *data_ptr = inspector_context_get_data(ctx);
    service_status_t *status = (service_status_t *)data_ptr;

    /* Initialize data */
    strcpy(status->name, "api-gateway");
    status->version.major = 1;
    status->version.minor = 2;
    status->version.patch = 3;
    status->cpu_usage = 45.6f;
    status->memory_mb = 512;
    status->connections = 1024;

    printf("1. Automatic JSON-like serialization using inspector_context API:\n\n");
    serialize_context_json(ctx, 0);

    printf("\n2. Demonstrate: swap to different service, same serializer:\n");

    service_status_t another_service = {
        .name = "database",
        .version = {.major = 5, .minor = 7, .patch = 10},
        .cpu_usage = 78.3f,
        .memory_mb = 2048,
        .connections = 256};

    /* Use inspector_context_update to swap data */
    inspector_context_update(ctx, &another_service);

    printf("\n");
    serialize_context_json(ctx, 0);

    printf("\n3. Show discovered metadata (name_at/type_at iteration):\n");
    printf("   Fields in service_status_t:\n");

    for (int i = 0; i < inspector_context_size(ctx); i++)
    {
        const char *name = inspector_context_name_at(ctx, i);
        const char *type = inspector_context_type_at(ctx, i);
        printf("   [%d] %s : %s\n", i, name, type);
    }

    printf("\n=== GENERIC SERIALIZATION PATTERN ===\n");
    printf("- Works on ANY struct that has registered metadata\n");
    printf("- Uses inspector_context_size(), name_at(), type_at(), get()\n");
    printf("- No per-type code - single path for all structs\n");
    printf("- Type information drives serialization logic\n");
    printf("- Safe offset-based field access via inspector_context_get()\n");
    printf("- Easy to extend: JSON/XML/protobuf converters\n");

    printf("\nCombined with API usage examples:\n");
    printf("- api_usage_validation.c: Type checking before use\n");
    printf("- api_usage_iteration.c: Field enumeration\n");
    printf("- api_usage_update.c: Safe data replacement\n");
    printf("- serialization_example.c: GENERIC OUTPUT (this file)\n");

    /* Cleanup */
    inspector_context_destroy(ctx);

    return 0;
}
