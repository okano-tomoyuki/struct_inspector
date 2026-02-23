/*!
 * @file arrays_struct.c
 * @brief Example demonstrating array and multi-dimensional array handling.
 *
 * Shows how the DSL macros register array field information including:
 * - 1D arrays (vectors)
 * - 2D arrays (matrices)
 * - Access via dimension metadata
 */

#include <stdio.h>
#include <string.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Structs with arrays ===== */

typedef struct {
    int values[10];
    char name[50];
} vector_data_t;

REGISTER_STRUCT_INFO_BEGIN(vector_data_t)
    REGISTER_FIELD_ARRAY(vector_data_t, values, int, NULL, 1, 10)
    REGISTER_FIELD_ARRAY(vector_data_t, name, char, NULL, 1, 50)
REGISTER_STRUCT_INFO_END(vector_data_t)

typedef struct {
    double matrix[3][4];
    int rows;
    int cols;
} matrix_data_t;

REGISTER_STRUCT_INFO_BEGIN(matrix_data_t)
    REGISTER_FIELD_ARRAY(matrix_data_t, matrix, double, NULL, 2, 3, 4)
    REGISTER_FIELD_SCALAR(matrix_data_t, rows, int, NULL)
    REGISTER_FIELD_SCALAR(matrix_data_t, cols, int, NULL)
REGISTER_STRUCT_INFO_END(matrix_data_t)

/* ===== Helper functions ===== */

static void print_array_info(const inspector_field_info_t* field)
{
    printf("  Field: %s\n", field->name);
    printf("    Type: %s\n", field->type_name);
    printf("    Dimensions: %zu\n", field->dim_count);
    if (field->dims && field->dim_count > 0) {
        printf("    Dimension sizes: [");
        for (size_t i = 0; i < field->dim_count; i++) {
            if (i > 0) printf(", ");
            printf("%zu", field->dims[i]);
        }
        printf("]\n");
    }
    printf("    Total size: %zu bytes\n", field->size);
}

/* ===== Main ===== */

int main(void)
{
    printf("=== Arrays Example ===\n\n");

    /* ===== Part 1: 1D Array ===== */
    printf("--- Part 1: 1D Array (Vector) ---\n");

    inspector_context_t* ctx1 = inspector_context_create(&vector_data_t_info, "vec");
    if (!ctx1) {
        fprintf(stderr, "Error: Failed to create inspector context\n");
        return 1;
    }

    vector_data_t vec_data;
    strcpy(vec_data.name, "fibonacci");
    for (int i = 0; i < 10; i++) {
        vec_data.values[i] = i * i;  /* 0, 1, 4, 9, 16, ... */
    }

    inspector_context_update(ctx1, &vec_data);
    vector_data_t* vec_ptr = (vector_data_t*)inspector_context_get_data(ctx1);

    printf("Vector data: %s\n", vec_ptr->name);
    printf("Values: [");
    for (int i = 0; i < 10; i++) {
        if (i > 0) printf(", ");
        printf("%d", vec_ptr->values[i]);
    }
    printf("]\n\n");

    /* Display array metadata */
    printf("Array field information:\n");
    for (size_t i = 0; i < vector_data_t_info.field_count; i++) {
        if (vector_data_t_info.fields[i].dim_count > 0) {
            print_array_info(&vector_data_t_info.fields[i]);
            printf("\n");
        }
    }

    inspector_context_destroy(ctx1);

    /* ===== Part 2: 2D Array (Matrix) ===== */
    printf("--- Part 2: 2D Array (Matrix) ---\n");

    inspector_context_t* ctx2 = inspector_context_create(&matrix_data_t_info, "mat");
    if (!ctx2) {
        fprintf(stderr, "Error: Failed to create inspector context\n");
        return 1;
    }

    matrix_data_t mat_data = {.rows = 3, .cols = 4};

    /* Fill matrix with sample data */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            mat_data.matrix[i][j] = i * 4.0 + j + 1.0;  /* 1.0, 2.0, 3.0, 4.0, 5.0, ... */
        }
    }

    inspector_context_update(ctx2, &mat_data);
    matrix_data_t* mat_ptr = (matrix_data_t*)inspector_context_get_data(ctx2);

    printf("Matrix (%d x %d):\n", mat_ptr->rows, mat_ptr->cols);
    for (int i = 0; i < 3; i++) {
        printf("  [");
        for (int j = 0; j < 4; j++) {
            if (j > 0) printf(", ");
            printf("%5.1f", mat_ptr->matrix[i][j]);
        }
        printf("]\n");
    }
    printf("\n");

    /* Display matrix array metadata */
    printf("Matrix field information:\n");
    for (size_t i = 0; i < matrix_data_t_info.field_count; i++) {
        if (matrix_data_t_info.fields[i].dim_count > 1) {
            print_array_info(&matrix_data_t_info.fields[i]);
            printf("\n");
        }
    }

    /* Compute strides based on dimension information */
    const inspector_field_info_t* matrix_field = &matrix_data_t_info.fields[0];
    if (matrix_field->dim_count == 2 && matrix_field->dims) {
        size_t dim_0 = matrix_field->dims[0];
        size_t dim_1 = matrix_field->dims[1];
        printf("Row-major stride calculation:\n");
        printf("  Dimensions: [%zu][%zu]\n", dim_0, dim_1);
        printf("  Element count: %zu\n", dim_0 * dim_1);
        printf("  Total bytes: %zu\n", dim_0 * dim_1 * sizeof(double));
    }

    inspector_context_destroy(ctx2);
    printf("\nCleanup complete.\n");

    return 0;
}
