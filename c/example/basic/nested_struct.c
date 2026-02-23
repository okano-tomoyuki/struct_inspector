/*!
 * @file nested_struct.c
 * @brief Example demonstrating nested struct introspection.
 *
 * This example shows how to work with structures containing other structs
 * as members. The DSL macros track nested struct metadata automatically
 * via the `nested` field in inspector_field_info_t.
 */

#include <stdio.h>
#include <string.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Struct hierarchy ===== */

typedef struct
{
    int x;
    int y;
} point_t;

REGISTER_STRUCT_INFO_BEGIN(point_t)
REGISTER_FIELD_SCALAR(point_t, x, int, NULL)
REGISTER_FIELD_SCALAR(point_t, y, int, NULL)
REGISTER_STRUCT_INFO_END(point_t)

typedef struct
{
    char label[32];
    point_t top_left;
    point_t bottom_right;
    int color;
} rectangle_t;

REGISTER_STRUCT_INFO_BEGIN(rectangle_t)
REGISTER_FIELD_ARRAY(rectangle_t, label, char, NULL, 1, 32)
REGISTER_FIELD_SCALAR(rectangle_t, top_left, point_t, &point_t_info)
REGISTER_FIELD_SCALAR(rectangle_t, bottom_right, point_t, &point_t_info)
REGISTER_FIELD_SCALAR(rectangle_t, color, int, NULL)
REGISTER_STRUCT_INFO_END(rectangle_t)

/* ===== Helper to print nested struct info ===== */

static void print_nested_info(const inspector_struct_info_t *info, int indent)
{
    for (size_t i = 0; i < info->field_count; i++)
    {
        const inspector_field_info_t *field = &info->fields[i];
        printf("%*s- %s: %s", indent, "", field->name, field->type_name);

        if (field->nested)
        {
            printf(" (nested struct with %zu fields)\n", field->nested->field_count);
            print_nested_info(field->nested, indent + 4);
        }
        else
        {
            printf("\n");
        }
    }
}

/* ===== Main ===== */

int main(void)
{
    printf("=== Nested Structures Example ===\n\n");

    /* Create context for rectangle_t */
    inspector_context_t *ctx = inspector_context_create(&rectangle_t_info, "rect");
    if (!ctx)
    {
        fprintf(stderr, "Error: Failed to create inspector context\n");
        return 1;
    }

    /* Initialize data with nested structs */
    rectangle_t rect = {
        .label = "Window A",
        .top_left = {10, 10},
        .bottom_right = {100, 80},
        .color = 0xFF5733};

    if (inspector_context_update(ctx, &rect) != 0)
    {
        fprintf(stderr, "Error: Failed to update context\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* Access data */
    rectangle_t *data = (rectangle_t *)inspector_context_get_data(ctx);
    if (!data)
    {
        fprintf(stderr, "Error: Failed to get data\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* Display data */
    printf("Rectangle data:\n");
    printf("  Label: %s\n", data->label);
    printf("  Top-left corner: (%d, %d)\n", data->top_left.x, data->top_left.y);
    printf("  Bottom-right corner: (%d, %d)\n", data->bottom_right.x, data->bottom_right.y);
    printf("  Color: 0x%06X\n\n", data->color);

    /* Inspect metadata hierarchy */
    printf("Struct hierarchy:\n");
    print_nested_info(&rectangle_t_info, 0);

    printf("\n");

    /* Demonstrate nested field access via metadata */
    printf("Nested field inspection:\n");
    for (size_t i = 0; i < rectangle_t_info.field_count; i++)
    {
        const inspector_field_info_t *field = &rectangle_t_info.fields[i];
        if (field->nested && strcmp(field->name, "top_left") == 0)
        {
            printf("  Field '%s' contains:\n", field->name);
            for (size_t j = 0; j < field->nested->field_count; j++)
            {
                const inspector_field_info_t *nested_field = &field->nested->fields[j];
                printf("    - %s (offset: %zu within nested, global: %zu)\n",
                       nested_field->name,
                       nested_field->offset,
                       field->offset + nested_field->offset);
            }
        }
    }

    /* Modify nested data */
    printf("\nModifying nested data...\n");
    data->bottom_right.x = 150;
    data->bottom_right.y = 100;
    printf("New bottom-right: (%d, %d)\n", data->bottom_right.x, data->bottom_right.y);

    /* Clean up */
    inspector_context_destroy(ctx);
    printf("\nCleanup complete.\n");

    return 0;
}
