/*!
 * @file api_usage_iteration.c
 * @brief Using inspector_context API to iterate and inspect struct fields.
 *
 * Demonstrates inspector_context_name_at(), inspector_context_type_at(),
 * and iteration patterns for discovering and working with struct members
 * at runtime.
 *
 * Real-world scenario: Building generic inspection tools, debug helpers,
 * or configuration UI generators that work with any struct.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Data Structure: User Profile ===== */

typedef struct
{
    int year;
    int month;
    int day;
} date_t;

REGISTER_STRUCT_INFO_BEGIN(date_t)
REGISTER_FIELD_SCALAR(date_t, year, int, NULL)
REGISTER_FIELD_SCALAR(date_t, month, int, NULL)
REGISTER_FIELD_SCALAR(date_t, day, int, NULL)
REGISTER_STRUCT_INFO_END(date_t)

typedef struct
{
    int user_id;
    char username[32];
    char email[64];
    date_t join_date;
    int reputation;
} user_profile_t;

REGISTER_STRUCT_INFO_BEGIN(user_profile_t)
REGISTER_FIELD_SCALAR(user_profile_t, user_id, int, NULL)
REGISTER_FIELD_ARRAY(user_profile_t, username, char, NULL, 1, 32)
REGISTER_FIELD_ARRAY(user_profile_t, email, char, NULL, 1, 64)
REGISTER_FIELD_SCALAR(user_profile_t, join_date, date_t, &date_t_info)
REGISTER_FIELD_SCALAR(user_profile_t, reputation, int, NULL)
REGISTER_STRUCT_INFO_END(user_profile_t)

/* ===== Generic field printer (works with any type) ===== */

static void print_field_value(const void *field_ptr, const char *type_name)
{
    if (strcmp(type_name, "int") == 0)
    {
        printf("%d", *(const int *)field_ptr);
    }
    else if (strcmp(type_name, "float") == 0)
    {
        printf("%.2f", *(const float *)field_ptr);
    }
    else if (strcmp(type_name, "double") == 0)
    {
        printf("%.6f", *(const double *)field_ptr);
    }
    else if (strcmp(type_name, "char") == 0)
    {
        printf("%s", (const char *)field_ptr);
    }
    else
    {
        printf("<complex type: %s>", type_name);
    }
}

/* ===== Generic dump function using name_at/type_at iteration ===== */

static void dump_struct_generic(inspector_context_t *ctx)
{
    int field_count = inspector_context_size(ctx);
    printf("{\n");

    for (int i = 0; i < field_count; i++)
    {
        /* Use inspector_context_name_at API */
        const char *field_name = inspector_context_name_at(ctx, i);
        if (!field_name)
            continue;

        /* Use inspector_context_type_at API */
        const char *field_type = inspector_context_type_at(ctx, i);
        if (!field_type)
            continue;

        printf("  %s: ", field_name);

        /* Access field using inspector_context_get */
        void *field_ptr = inspector_context_get(ctx, field_name);
        if (field_ptr)
        {
            print_field_value(field_ptr, field_type);
        }
        else
        {
            printf("<error>");
        }

        printf("\n");
    }

    printf("}\n");
}

/* ===== Helper: Compare two struct instances for delta ===== */

static void show_field_diff(
    inspector_context_t *ctx,
    const void *original_data)
{
    int field_count = inspector_context_size(ctx);
    int changes = 0;

    /* Get current data from context */
    const void *current_data = inspector_context_get_data(ctx);

    printf("Changes detected:\n");

    for (int i = 0; i < field_count; i++)
    {
        const char *field_name = inspector_context_name_at(ctx, i);
        const char *field_type = inspector_context_type_at(ctx, i);
        if (!field_name || !field_type)
            continue;

        const void *ptr1 = (const char *)original_data +
                           ((const char *)inspector_context_get(ctx, field_name) -
                            (const char *)current_data);

        void *ptr2 = inspector_context_get(ctx, field_name);

        if (!ptr2)
            continue;

        /* Simple byte comparison (works for scalars) */
        if (strcmp(field_type, "int") == 0)
        {
            int v1 = *(const int *)original_data;
            int v2 = *(int *)ptr2;
            if (v1 != v2)
            {
                printf("  '%s': changed\n", field_name);
                changes++;
            }
        }
        else if (strcmp(field_type, "char") == 0)
        {
            if (strcmp((const char *)original_data, (const char *)ptr2) != 0)
            {
                printf("  '%s': changed\n", field_name);
                changes++;
            }
        }
    }

    if (changes == 0)
    {
        printf("  (no changes)\n");
    }
}

/* ===== Main ===== */

int main(void)
{
    printf("=== Inspector Context API: Generic Iteration & Introspection ===\n\n");

    /* Create context with user profile */
    inspector_context_t *ctx = inspector_context_create(
        &user_profile_t_info,
        "user");

    const void *data_ptr = inspector_context_get_data(ctx);
    user_profile_t *profile = (user_profile_t *)data_ptr;

    profile->user_id = 12345;
    strcpy(profile->username, "alice_dev");
    strcpy(profile->email, "alice@example.com");
    profile->join_date.year = 2023;
    profile->join_date.month = 6;
    profile->join_date.day = 15;
    profile->reputation = 850;

    printf("1. Generic struct dump (using name_at/type_at iteration):\n");
    dump_struct_generic(ctx);

    printf("\n2. Iterate all fields and show metadata (no hardcoding):\n");
    int field_count = inspector_context_size(ctx);
    printf("   Total fields: %d\n", field_count);

    for (int i = 0; i < field_count; i++)
    {
        const char *name = inspector_context_name_at(ctx, i);
        const char *type = inspector_context_type_at(ctx, i);
        printf("   [%d] %s : %s\n", i, name ? name : "?", type ? type : "?");
    }

    printf("\n3. Field existence check (using contains()):\n");
    const char *fields_to_check[] = {
        "user.user_id",
        "user.username",
        "user.join_date",
        "user.role",
        "user.permissions"
    };

    for (int i = 0; i < 5; i++)
    {
        const char *field = fields_to_check[i];
        int exists = inspector_context_contains(ctx, field);
        printf("   '%s': %s\n", field, exists ? "exists [o]" : "missing [x]");
    }

    printf("\n4. Safe type-based field access pattern:\n");
    printf("   Accessing each field by index and type:\n");

    for (int i = 0; i < field_count; i++)
    {
        const char *field_name = inspector_context_name_at(ctx, i);
        const char *field_type = inspector_context_type_at(ctx, i);

        if (!field_name || !field_type)
            continue;

        printf("   Field[%d]: '%s' (type='%s')\n", i, field_name, field_type);

        /* Type-based conditional logic without manual switch */
        if (strcmp(field_type, "int") == 0)
        {
            void *pval = inspector_context_get(ctx, field_name);
            if (pval)
                printf("      Value: %d\n", *(int *)pval);
        }
        else if (strcmp(field_type, "char") == 0)
        {
            void *pval = inspector_context_get(ctx, field_name);
            if (pval)
                printf("      Value: %s\n", (char *)pval);
        }
    }

    printf("\n5. Generic serialization pattern (works ANY struct):\n");
    printf("   Pseudo-JSON generated from metadata:\n");
    printf("   {\n");
    for (int i = 0; i < field_count; i++)
    {
        const char *name = inspector_context_name_at(ctx, i);
        const char *type = inspector_context_type_at(ctx, i);
        if (name && type)
        {
            printf("     \"%s\": <%s>%s\n", name, type,
                   i < field_count - 1 ? "," : "");
        }
    }
    printf("   }\n");

    printf("\n=== KEY BENEFITS ===\n");
    printf("- inspector_context_name_at() - get field name by index\n");
    printf("- inspector_context_type_at() - get field type by index\n");
    printf("- inspector_context_size() - loop over ALL fields\n");
    printf("- inspector_context_get() - safe offset-based access\n");
    printf("- inspector_context_contains() - field existence check\n");
    printf("\nCombined power:\n");
    printf("- Generic dump/debug functions (works on ANY struct)\n");
    printf("- Automatic schema introspection\n");
    printf("- Field-level iteration without hardcoding\n");
    printf("- Single code path handles multiple struct types\n");

    /* Cleanup */
    inspector_context_destroy(ctx);

    return 0;
}
