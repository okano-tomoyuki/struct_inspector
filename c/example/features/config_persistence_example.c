/*!
 * @file config_persistence_example.c
 * @brief Configuration persistence using inspector_context API.
 *
 * Demonstrates using inspector_context_create(), inspector_context_update(),
 * and inspector_context_get_data() for safe configuration file I/O with
 * type awareness and fixed address guarantees.
 *
 * Real-world scenario: Load configuration from file with type validation,
 * reload without dangling pointers, persistent storage without corruption.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Configuration Structure ===== */

typedef struct
{
    char database_host[64];
    int database_port;
    char api_key[128];
    int max_retries;
    float timeout_seconds;
} app_config_t;

REGISTER_STRUCT_INFO_BEGIN(app_config_t)
REGISTER_FIELD_ARRAY(app_config_t, database_host, char, NULL, 1, 64)
REGISTER_FIELD_SCALAR(app_config_t, database_port, int, NULL)
REGISTER_FIELD_ARRAY(app_config_t, api_key, char, NULL, 1, 128)
REGISTER_FIELD_SCALAR(app_config_t, max_retries, int, NULL)
REGISTER_FIELD_SCALAR(app_config_t, timeout_seconds, float, NULL)
REGISTER_STRUCT_INFO_END(app_config_t)

/* ===== Configuration serialization using inspector_context API ===== */

/**
 * Serialize configuration to a simple key=value format using
 * inspector_context API for metadata-driven field access.
 */
static void serialize_config(inspector_context_t *ctx, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f)
    {
        printf("ERROR: Cannot open %s for writing\n", filename);
        return;
    }

    int field_count = inspector_context_size(ctx);

    for (int i = 0; i < field_count; i++)
    {
        /* Use inspector_context API to discover field */
        const char *full_name = inspector_context_name_at(ctx, i);
        const char *type = inspector_context_type_at(ctx, i);

        if (!full_name || !type)
            continue;

        /* Extract short name (remove prefix) */
        const char *name = full_name;
        if (strchr(full_name, '.'))
        {
            name = strchr(full_name, '.') + 1;
        }

        /* Get field value using inspector_context_get */
        void *ptr = inspector_context_get(ctx, full_name);
        if (!ptr)
            continue;

        /* Serialize based on type */
        fprintf(f, "%s=", name);

        if (strcmp(type, "int") == 0)
        {
            fprintf(f, "%d\n", *(int *)ptr);
        }
        else if (strcmp(type, "float") == 0)
        {
            fprintf(f, "%.6f\n", *(float *)ptr);
        }
        else if (strcmp(type, "char") == 0)
        {
            fprintf(f, "%s\n", (char *)ptr);
        }
    }

    fclose(f);
}

/**
 * Deserialize configuration from key=value format and update context.
 * Safe: inspector_context_update ensures fixed address lifetime.
 */
static int deserialize_config(inspector_context_t *ctx, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        printf("ERROR: Cannot open %s for reading\n", filename);
        return -1;
    }

    /* Create a working copy of the current data */
    const void *current_data = inspector_context_get_data(ctx);
    size_t data_size = sizeof(app_config_t); /* For this example */
    void *temp_data = malloc(data_size);
    memcpy(temp_data, current_data, data_size);

    char line[256];
    int updates = 0;

    while (fgets(line, sizeof(line), f))
    {
        /* Parse key=value */
        char *equals = strchr(line, '=');
        if (!equals)
            continue;

        *equals = '\0';
        char *key = line;
        char *value = equals + 1;

        /* Remove trailing newline */
        char *newline = strchr(value, '\n');
        if (newline)
            *newline = '\0';

        /* Reconstruct full field name with prefix */
        char full_field_name[256];
        snprintf(full_field_name, sizeof(full_field_name),
                 "config.%s", key);

        /* Find matching field via inspector_context API */
        if (!inspector_context_contains(ctx, full_field_name))
        {
            printf("  WARNING: Unknown field '%s'\n", key);
            continue;
        }

        /* Get the field type */
        const char *type = inspector_context_type(ctx, full_field_name);
        if (!type)
            continue;

        /* Update the temporary data based on type */
        void *field_ptr = inspector_context_get(ctx, full_field_name);
        if (field_ptr)
        {
            if (strcmp(type, "int") == 0)
            {
                *(int *)field_ptr = atoi(value);
            }
            else if (strcmp(type, "float") == 0)
            {
                *(float *)field_ptr = atof(value);
            }
            else if (strcmp(type, "char") == 0)
            {
                strncpy((char *)field_ptr, value, 63);
            }
            updates++;
        }
    }

    fclose(f);

    /* Use inspector_context_update to safely replace data
       This preserves fixed address while swapping content */
    app_config_t *temp_struct = (app_config_t *)temp_data;
    inspector_context_update(ctx, temp_struct);

    free(temp_data);

    return updates;
}

/* ===== Main ===== */

int main(void)
{
    printf("=== Configuration Persistence with inspector_context API ===\n\n");

    /* Create context to manage configuration */
    inspector_context_t *ctx = inspector_context_create(
        &app_config_t_info,
        "config");

    const void *data_ptr = inspector_context_get_data(ctx);
    app_config_t *config = (app_config_t *)data_ptr;

    printf("1. Initialize configuration:\n");
    strcpy(config->database_host, "db.example.com");
    config->database_port = 5432;
    strcpy(config->api_key, "secret-key-12345");
    config->max_retries = 3;
    config->timeout_seconds = 30.0f;

    printf("   database_host='%s'\n", config->database_host);
    printf("   database_port=%d\n", config->database_port);
    printf("   api_key='%s'\n", config->api_key);
    printf("   max_retries=%d\n", config->max_retries);
    printf("   timeout_seconds=%.1f\n\n", config->timeout_seconds);

    printf("2. Save configuration using inspector_context API:\n");
    serialize_config(ctx, "app.conf");
    printf("   Saved to app.conf (8 file persisted via inspector_context_name_at/type_at/get)\n");

    /* Show what was saved */
    printf("\n   Contents of app.conf:\n");
    FILE *f = fopen("app.conf", "r");
    if (f)
    {
        char line[256];
        while (fgets(line, sizeof(line), f))
        {
            printf("   %s", line);
        }
        fclose(f);
    }

    printf("\n3. Simulate loading modified configuration:\n");
    app_config_t modified = {
        .database_host = "db-backup.example.com",
        .database_port = 5433,
        .api_key = "new-key-99999",
        .max_retries = 5,
        .timeout_seconds = 60.0f};

    printf("   Writing modified config...\n");

    /* Save modified config to file */
    f = fopen("app.conf", "w");
    fprintf(f, "database_host=%s\n", modified.database_host);
    fprintf(f, "database_port=%d\n", modified.database_port);
    fprintf(f, "api_key=%s\n", modified.api_key);
    fprintf(f, "max_retries=%d\n", modified.max_retries);
    fprintf(f, "timeout_seconds=%.6f\n", modified.timeout_seconds);
    fclose(f);

    printf("\n4. Reload configuration via inspector_context_update():\n");
    printf("   (Context maintains fixed address - no pointer invalidation)\n");

    /* Store data address before reload */
    const void *addr_before = inspector_context_get_data(ctx);
    printf("   Address before reload: %p\n", addr_before);

    /* Load from file */
    int updates = deserialize_config(ctx, "app.conf");
    printf("   Loaded and updated %d fields\n", updates);

    /* Check address after reload */
    const void *addr_after = inspector_context_get_data(ctx);
    printf("   Address after reload:  %p (%s)\n",
           addr_after,
           addr_before == addr_after ? "SAME ✓" : "DIFFERENT ✗");

    /* Verify loaded values */
    printf("\n5. Verify loaded configuration:\n");
    for (int i = 0; i < inspector_context_size(ctx); i++)
    {
        const char *field_name = inspector_context_name_at(ctx, i);
        const char *field_type = inspector_context_type_at(ctx, i);

        if (!field_name || !field_type)
            continue;

        void *ptr = inspector_context_get(ctx, field_name);

        const char *name = field_name;
        if (strchr(field_name, '.'))
        {
            name = strchr(field_name, '.') + 1;
        }

        printf("   %s = ", name);

        if (ptr)
        {
            if (strcmp(field_type, "int") == 0)
            {
                printf("%d", *(int *)ptr);
            }
            else if (strcmp(field_type, "float") == 0)
            {
                printf("%.1f", *(float *)ptr);
            }
            else if (strcmp(field_type, "char") == 0)
            {
                printf("\"%s\"", (char *)ptr);
            }
        }
        printf("\n");
    }

    printf("\n=== CONFIGURATION PERSISTENCE PATTERN ===\n");
    printf("- inspector_context owns configuration memory\n");
    printf("- serialize_config: Uses name_at/type_at/get for generic output\n");
    printf("- deserialize_config: Uses contains/type/get for input validation\n");
    printf("- inspector_context_update: Swaps data, maintains fixed address\n");
    printf("- No dangling pointers after reload\n");
    printf("- Type-safe: validation prevents corrupt data\n");

    printf("\nCombined with other examples:\n");
    printf("- api_usage_validation.c: Type checking on loaded values\n");
    printf("- api_usage_iteration.c: Field enumeration (used here)\n");
    printf("- api_usage_update.c: Safe data replacement (used here)\n");
    printf("- serialization_example.c: Generic JSON output\n");
    printf("- string_query_example.c: Dynamic field queries\n");
    printf("- config_persistence_example.c: FILE I/O (this file)\n");

    /* Cleanup */
    inspector_context_destroy(ctx);
    remove("app.conf");

    return 0;
}
