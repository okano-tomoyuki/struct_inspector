/*!
 * @file basic_struct.c
 * @brief Basic usage example with simple scalar types.
 *
 * This example demonstrates the fundamental workflow:
 * - Define a simple struct with scalar members
 * - Register metadata using DSL macros
 * - Create an inspector context
 * - Access and modify data safely
 */

#include <stdio.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Simple struct definition ===== */

typedef struct {
    int id;
    float temperature;
    double pressure;
} sensor_reading_t;

/* Register metadata for sensor_reading_t using DSL macros */
REGISTER_STRUCT_INFO_BEGIN(sensor_reading_t)
    REGISTER_FIELD_SCALAR(sensor_reading_t, id, int, NULL)
    REGISTER_FIELD_SCALAR(sensor_reading_t, temperature, float, NULL)
    REGISTER_FIELD_SCALAR(sensor_reading_t, pressure, double, NULL)
REGISTER_STRUCT_INFO_END(sensor_reading_t)

/* ===== Main ===== */

int main(void)
{
    printf("=== Basic Inspector Example ===\n\n");

    /* Create an inspector context for sensor_reading_t with prefix "sensor" */
    inspector_context_t* ctx = inspector_context_create(&sensor_reading_t_info, "sensor");
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create inspector context\n");
        return 1;
    }

    /* Initialize data */
    sensor_reading_t reading = {
        .id = 1001,
        .temperature = 23.5f,
        .pressure = 101.325
    };

    /* Copy data into context's owned memory */
    if (inspector_context_update(ctx, &reading) != 0) {
        fprintf(stderr, "Error: Failed to update context\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* Get stable pointer to owned data */
    sensor_reading_t* data = (sensor_reading_t*)inspector_context_get_data(ctx);
    if (!data) {
        fprintf(stderr, "Error: Failed to get data pointer\n");
        inspector_context_destroy(ctx);
        return 1;
    }

    /* Display initial data */
    printf("Initial sensor reading:\n");
    printf("  ID: %d\n", data->id);
    printf("  Temperature: %.2f°C\n", data->temperature);
    printf("  Pressure: %.3f hPa\n\n", data->pressure);

    /* Modify data through the owned pointer */
    data->temperature = 24.8f;
    data->pressure = 101.450;

    printf("Modified sensor reading:\n");
    printf("  ID: %d\n", data->id);
    printf("  Temperature: %.2f°C\n", data->temperature);
    printf("  Pressure: %.3f hPa\n\n", data->pressure);

    /* Query metadata using inspector wrappers */
    printf("Struct metadata:\n");
    printf("  Struct name: %s\n", sensor_reading_t_info.struct_name);
    printf("  Struct size: %zu bytes\n", sensor_reading_t_info.size);
    printf("  Field count: %zu\n\n", sensor_reading_t_info.field_count);

    /* Iterate through fields */
    printf("Fields:\n");
    for (size_t i = 0; i < sensor_reading_t_info.field_count; i++) {
        const inspector_field_info_t* field = &sensor_reading_t_info.fields[i];
        printf("  %zu. %s (type: %s, offset: %zu, size: %zu)\n",
               i + 1,
               field->name,
               field->type_name,
               field->offset,
               field->size);
    }

    /* Clean up */
    inspector_context_destroy(ctx);
    printf("\nCleanup complete.\n");

    return 0;
}
