/*!
 * @file test_dsl.c
 * @brief Unit tests for DSL metadata generation and binding.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Enable DSL macros before including the header */
#define ENABLE_INSPECTOR_BIND_DSL

#include "inspector_dsl.h"

/* ====== Test Framework ====== */

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT_TRUE(condition, message)                        \
    do                                                         \
    {                                                          \
        g_tests_run++;                                         \
        if (!(condition))                                      \
        {                                                      \
            printf("FAIL: %s (line %d)\n", message, __LINE__); \
            g_tests_failed++;                                  \
        }                                                      \
        else                                                   \
        {                                                      \
            printf("PASS: %s\n", message);                     \
            g_tests_passed++;                                  \
        }                                                      \
    } while (0)

#define ASSERT_EQUAL(actual, expected, message) \
    ASSERT_TRUE((actual) == (expected), message)

#define ASSERT_NULL(ptr, message) \
    ASSERT_TRUE((ptr) == NULL, message)

#define ASSERT_NOT_NULL(ptr, message) \
    ASSERT_TRUE((ptr) != NULL, message)

#define ASSERT_STR_EQUAL(str1, str2, message) \
    ASSERT_TRUE(strcmp((str1), (str2)) == 0, message)

/* ====== Test Structs and DSL Registration ====== */

typedef struct
{
    int x;
    float y;
} point_t;

REGISTER_STRUCT_INFO_BEGIN(point_t)
REGISTER_FIELD_SCALAR(point_t, x, int, NULL)
REGISTER_FIELD_SCALAR(point_t, y, float, NULL)
REGISTER_STRUCT_INFO_END(point_t)

typedef struct
{
    char name[32];
    int arr[5];
    double matrix[3][4];
} container_t;

REGISTER_STRUCT_INFO_BEGIN(container_t)
REGISTER_FIELD_ARRAY(container_t, name, char, NULL, 1, 32)
REGISTER_FIELD_ARRAY(container_t, arr, int, NULL, 1, 5)
REGISTER_FIELD_ARRAY(container_t, matrix, double, NULL, 2, 3, 4)
REGISTER_STRUCT_INFO_END(container_t)

typedef struct
{
    point_t p;
    int id;
} with_nested_t;

REGISTER_STRUCT_INFO_BEGIN(with_nested_t)
REGISTER_FIELD_SCALAR(with_nested_t, p, point_t, &point_t_info)
REGISTER_FIELD_SCALAR(with_nested_t, id, int, NULL)
REGISTER_STRUCT_INFO_END(with_nested_t)

/* ====== Tests ====== */

static void test_struct_info_metadata(void)
{
    printf("\n=== test_struct_info_metadata ===\n");

    ASSERT_NOT_NULL(point_t_info.struct_name, "struct_name not NULL");
    ASSERT_STR_EQUAL(point_t_info.struct_name, "point_t", "struct_name is \"point_t\"");

    ASSERT_EQUAL(point_t_info.size, sizeof(point_t), "size matches sizeof");
    ASSERT_EQUAL(point_t_info.field_count, 2, "field_count is 2");
    ASSERT_NOT_NULL(point_t_info.fields, "fields array not NULL");
}

static void test_field_info_scalar(void)
{
    printf("\n=== test_field_info_scalar ===\n");

    const inspector_field_info_t *f = &point_t_info.fields[0];
    ASSERT_STR_EQUAL(f->name, "x", "field name is \"x\"");
    ASSERT_STR_EQUAL(f->type_name, "int", "type_name is \"int\"");
    ASSERT_EQUAL(f->size, sizeof(int), "field size is sizeof(int)");
    ASSERT_EQUAL(f->offset, offsetof(point_t, x), "offset matches offsetof");
    ASSERT_EQUAL(f->dim_count, 0, "dim_count is 0 for scalar");
    ASSERT_NULL(f->dims, "dims is NULL for scalar");
    ASSERT_NULL(f->nested, "nested is NULL for non-struct scalar");
}

static void test_field_info_array(void)
{
    printf("\n=== test_field_info_array ===\n");

    const inspector_field_info_t *f_arr = NULL;
    for (int i = 0; i < container_t_info.field_count; i++)
    {
        if (strcmp(container_t_info.fields[i].name, "arr") == 0)
        {
            f_arr = &container_t_info.fields[i];
            break;
        }
    }
    ASSERT_NOT_NULL(f_arr, "found 'arr' field");

    ASSERT_STR_EQUAL(f_arr->name, "arr", "field name is \"arr\"");
    ASSERT_EQUAL(f_arr->dim_count, 1, "dim_count is 1");
    ASSERT_NOT_NULL(f_arr->dims, "dims is not NULL");
    ASSERT_EQUAL(f_arr->dims[0], 5, "dims[0] is 5");
    ASSERT_EQUAL(f_arr->size, sizeof(int) * 5, "field size is sizeof(int)*5");
}

static void test_field_info_multidim_array(void)
{
    printf("\n=== test_field_info_multidim_array ===\n");

    const inspector_field_info_t *f_matrix = NULL;
    for (int i = 0; i < container_t_info.field_count; i++)
    {
        if (strcmp(container_t_info.fields[i].name, "matrix") == 0)
        {
            f_matrix = &container_t_info.fields[i];
            break;
        }
    }
    ASSERT_NOT_NULL(f_matrix, "found 'matrix' field");

    ASSERT_EQUAL(f_matrix->dim_count, 2, "dim_count is 2");
    ASSERT_NOT_NULL(f_matrix->dims, "dims is not NULL");
    ASSERT_EQUAL(f_matrix->dims[0], 3, "dims[0] is 3");
    ASSERT_EQUAL(f_matrix->dims[1], 4, "dims[1] is 4");
    ASSERT_EQUAL(f_matrix->size, sizeof(double) * 3 * 4, "size is 3*4*sizeof(double)");
}

static void test_nested_struct_info(void)
{
    printf("\n=== test_nested_struct_info ===\n");

    const inspector_field_info_t *f = NULL;
    for (int i = 0; i < with_nested_t_info.field_count; i++)
    {
        if (strcmp(with_nested_t_info.fields[i].name, "p") == 0)
        {
            f = &with_nested_t_info.fields[i];
            break;
        }
    }
    ASSERT_NOT_NULL(f, "found 'p' field");

    ASSERT_NOT_NULL(f->nested, "nested struct pointer is not NULL");
    ASSERT_STR_EQUAL(f->nested->struct_name, "point_t", "nested struct name is \"point_t\"");
    ASSERT_EQUAL(f->nested->field_count, 2, "nested field_count is 2");
}

static void test_field_offset_correctness(void)
{
    printf("\n=== test_field_offset_correctness ===\n");

    const inspector_field_info_t *fields = container_t_info.fields;

    for (int i = 0; i < container_t_info.field_count; i++)
    {
        const inspector_field_info_t *f = &fields[i];

        if (strcmp(f->name, "name") == 0)
        {
            ASSERT_EQUAL(f->offset, offsetof(container_t, name), "name offset correct");
        }
        else if (strcmp(f->name, "arr") == 0)
        {
            ASSERT_EQUAL(f->offset, offsetof(container_t, arr), "arr offset correct");
        }
        else if (strcmp(f->name, "matrix") == 0)
        {
            ASSERT_EQUAL(f->offset, offsetof(container_t, matrix), "matrix offset correct");
        }
    }
}

static void test_postfix_configuration(void)
{
    printf("\n=== test_postfix_configuration ===\n");

    /* This test verifies that the configured postfix (_info) is applied.
     * The symbol should be named point_t_info (not point_t_data or similar).
     * If the postfix were wrong, the link would fail or this test couldn't
     * reference the symbol.
     */
    ASSERT_NOT_NULL(&point_t_info, "point_t_info symbol exists (correct postfix)");
    ASSERT_NOT_NULL(&container_t_info, "container_t_info symbol exists");
    printf("PASS: Generated symbols use configured postfix\n");
    g_tests_run++;
    g_tests_passed++;
}

/* ====== Main ====== */

int main(void)
{
    printf("=== DSL metadata generation tests ===\n");

    test_struct_info_metadata();
    test_field_info_scalar();
    test_field_info_array();
    test_field_info_multidim_array();
    test_nested_struct_info();
    test_field_offset_correctness();
    test_postfix_configuration();

    printf("\n=== Summary ===\n");
    printf("Tests run: %d\n", g_tests_run);
    printf("Passed: %d\n", g_tests_passed);
    printf("Failed: %d\n", g_tests_failed);

    if (g_tests_failed > 0)
    {
        printf("\nRESULT: FAILED\n");
        return 1;
    }
    else
    {
        printf("\nRESULT: ALL TESTS PASSED\n");
        return 0;
    }
}
