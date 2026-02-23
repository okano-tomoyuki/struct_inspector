/*!
 * @file string_query_example.c
 * @brief String-based path queries using inspector_context API.
 *
 * Demonstrates using inspector_context_contains(), inspector_context_get(),
 * inspector_context_type(), etc. to implement dot-notation path queries
 * like "config.firmware.major" safely at runtime.
 *
 * Real-world scenario: IPC/message parsing, configuration system, or
 * dynamic data access without hardcoded field logic.
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
} version_t;

REGISTER_STRUCT_INFO_BEGIN(version_t)
REGISTER_FIELD_SCALAR(version_t, major, int, NULL)
REGISTER_FIELD_SCALAR(version_t, minor, int, NULL)
REGISTER_STRUCT_INFO_END(version_t)

typedef struct
{
    version_t firmware;
    int build_number;
    char build_date[32];
} device_config_t;

REGISTER_STRUCT_INFO_BEGIN(device_config_t)
REGISTER_FIELD_SCALAR(device_config_t, firmware, version_t, &version_t_info)
REGISTER_FIELD_SCALAR(device_config_t, build_number, int, NULL)
REGISTER_FIELD_ARRAY(device_config_t, build_date, char, NULL, 1, 32)
REGISTER_STRUCT_INFO_END(device_config_t)

typedef struct
{
    char serial[32];
    device_config_t config;
    float temperature;
} device_state_t;

REGISTER_STRUCT_INFO_BEGIN(device_state_t)
REGISTER_FIELD_ARRAY(device_state_t, serial, char, NULL, 1, 32)
REGISTER_FIELD_SCALAR(device_state_t, config, device_config_t, &device_config_t_info)
REGISTER_FIELD_SCALAR(device_state_t, temperature, float, NULL)
REGISTER_STRUCT_INFO_END(device_state_t)

/* ===== Dynamic path query using inspector_context API ===== */

/**
 * Query a field using dot-notation path (e.g., "device.config.firmware.major").
 * Uses inspector_context_contains() and inspector_context_get() to safely navigate.
 */
typedef struct
{
    const void *ptr;
    const char *type_name;
    int found;
} query_result_t;

static query_result_t query_field_by_path(
    inspector_context_t *ctx,
    const char *dot_path)
{
    query_result_t result = {.ptr = NULL, .type_name = NULL, .found = 0};

    /* Try direct field access first (inspector_context uses full path with prefix) */
    if (inspector_context_contains(ctx, dot_path))
    {
        result.ptr = inspector_context_get(ctx, dot_path);
        result.type_name = inspector_context_type(ctx, dot_path);
        if (result.ptr)
        {
            result.found = 1;
        }
        return result;
    }

    /* Not found */
    return result;
}

/**
 * Enumerate all discoverable paths by iterating inspector_context API.
 */
static void enumerate_paths(inspector_context_t *ctx)
{
    int field_count = inspector_context_size(ctx);

    for (int i = 0; i < field_count; i++)
    {
        const char *name = inspector_context_name_at(ctx, i);
        const char *type = inspector_context_type_at(ctx, i);

        if (name && type)
        {
            printf("  - %s (%s)\n", name, type);
        }
    }
}

/**
 * Display a value based on its type name.
 */
static void print_value(const void *ptr, const char *type_name)
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
        printf("%s", (const char *)ptr);
    }
    else
    {
        printf("<type: %s>", type_name);
    }
}

/* ===== Main ===== */

int main(void)
{
    printf("=== String-based Path Queries with inspector_context API ===\n\n");

    /* Create context for device state */
    inspector_context_t *ctx = inspector_context_create(
        &device_state_t_info,
        "device");

    const void *data_ptr = inspector_context_get_data(ctx);
    device_state_t *device = (device_state_t *)data_ptr;

    strcpy(device->serial, "DEV-001-ABC");
    device->config.firmware.major = 2;
    device->config.firmware.minor = 5;
    device->config.build_number = 4731;
    strcpy(device->config.build_date, "2026-02-23");
    device->temperature = 42.3f;

    printf("1. Query nested fields using dot-notation paths (inspector_context API):\n");

    struct
    {
        const char *path;
    } queries[] = {
        {"device.serial"},
        {"device.temperature"},
        {"device.config.build_number"},
        {"device.config.build_date"},
        {"device.config.firmware.major"},
        {"device.config.firmware.minor"},
    };

    int num_queries = sizeof(queries) / sizeof(queries[0]);

    for (int i = 0; i < num_queries; i++)
    {
        const char *path = queries[i].path;
        query_result_t result = query_field_by_path(ctx, path);

        printf("  Query: \"%s\"\n", path);

        if (result.found)
        {
            printf("    Type: %s, Value: ", result.type_name);
            print_value(result.ptr, result.type_name);
            printf("\n");
        }
        else
        {
            printf("    NOT FOUND\n");
        }
    }

    printf("\n2. Failed query attempts (inspector_context_contains returns false):\n");

    const char *invalid_paths[] = {
        "device.nonexistent",
        "device.config.invalid_field",
        "device.config.firmware.nonexistent"};

    for (int i = 0; i < 3; i++)
    {
        const char *path = invalid_paths[i];
        int exists = inspector_context_contains(ctx, path);
        printf("  \"%s\": %s (inspector_context_contains)\n",
               path,
               exists ? "FOUND" : "NOT FOUND (expected)");
    }

    printf("\n3. Enumerate ALL discoverable paths (using name_at/type_at iteration):\n");
    enumerate_paths(ctx);

    printf("\n4. Dynamic member access without hardcoded logic:\n");

    int count = inspector_context_size(ctx);
    printf("   Total fields in device_state_t: %d\n", count);
    printf("   These can be accessed generically via:");
    printf("\n   inspector_context_contains(ctx, path)\n");
    printf("   inspector_context_get(ctx, path)\n");
    printf("   inspector_context_type(ctx, path)\n");

    printf("\n5. Type-safe conditional logic (no manual switch):\n");

    for (int i = 0; i < count; i++)
    {
        const char *name = inspector_context_name_at(ctx, i);
        const char *type = inspector_context_type_at(ctx, i);

        if (name && type && strcmp(type, "int") == 0)
        {
            void *ptr = inspector_context_get(ctx, name);
            if (ptr)
            {
                printf("   INT field '%s' = %d\n", name, *(int *)ptr);
            }
        }
    }

    printf("\n=== STRING-BASED QUERY PATTERN ===\n");
    printf("- inspector_context_contains() - check path existence\n");
    printf("- inspector_context_get() - safe offset-based lookup\n");
    printf("- inspector_context_type() - determine value type\n");
    printf("- inspector_context_name_at/type_at - enumerate fields\n");
    printf("\nBenefits:\n");
    printf("- Dynamic field access without hardcoding member names\n");
    printf("- Type-driven conditional logic\n");
    printf("- Works on ANY struct with metadata\n");
    printf("- Safe: validation via contains() before access\n");

    printf("\nCombined with other examples:\n");
    printf("- api_usage_validation.c: Real-time type checking\n");
    printf("- api_usage_iteration.c: Generic introspection\n");
    printf("- api_usage_update.c: Safe data replacement\n");
    printf("- string_query_example.c: DYNAMIC QUERIES (this file)\n");

    /* Cleanup */
    inspector_context_destroy(ctx);

    return 0;
}
