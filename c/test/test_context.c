/*!
 * @file test_context.c
 * @brief Unit tests for the inspector context wrapper.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector_context.h"

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
    int id;
    float value;
} simple_t;

REGISTER_STRUCT_INFO_BEGIN(simple_t)
REGISTER_FIELD_SCALAR(simple_t, id, int, NULL)
REGISTER_FIELD_SCALAR(simple_t, value, float, NULL)
REGISTER_STRUCT_INFO_END(simple_t)

typedef struct
{
    char name[64];
    simple_t nested;
} complex_t;

REGISTER_STRUCT_INFO_BEGIN(complex_t)
REGISTER_FIELD_ARRAY(complex_t, name, char, NULL, 1, 64)
REGISTER_FIELD_SCALAR(complex_t, nested, simple_t, &simple_t_info)
REGISTER_STRUCT_INFO_END(complex_t)

/* ====== Tests ====== */

static void test_create_destroy(void)
{
    printf("\n=== test_create_destroy ===\n");

    inspector_context_t *ctx = inspector_context_create(&simple_t_info, "test");
    ASSERT_NOT_NULL(ctx, "inspector_context_create returns non-NULL");

    inspector_context_destroy(ctx);
    printf("PASS: inspector_context_destroy completed without crash\n");
    g_tests_run++;
    g_tests_passed++;
}

static void test_get_data(void)
{
    printf("\n=== test_get_data ===\n");

    inspector_context_t *ctx = inspector_context_create(&simple_t_info, "test");
    ASSERT_NOT_NULL(ctx, "inspector_context_create");

    const void *data = inspector_context_get_data(ctx);
    ASSERT_NOT_NULL(data, "inspector_context_get_data returns non-NULL");

    inspector_context_destroy(ctx);
}

static void test_update(void)
{
    printf("\n=== test_update ===\n");

    inspector_context_t *ctx = inspector_context_create(&simple_t_info, "test");
    ASSERT_NOT_NULL(ctx, "inspector_context_create");

    simple_t source = {42, 3.14f};
    int ret = inspector_context_update(ctx, &source);
    ASSERT_EQUAL(ret, 0, "inspector_context_update returns 0 on success");

    const simple_t *owned = (const simple_t *)inspector_context_get_data(ctx);
    ASSERT_NOT_NULL(owned, "get_data after update");
    ASSERT_EQUAL(owned->id, 42, "id copied correctly");

    /* Verify it's stable memory: update again and check */
    const simple_t *first_ptr = owned;
    simple_t source2 = {99, 2.71f};
    inspector_context_update(ctx, &source2);
    const simple_t *second_ptr = (const simple_t *)inspector_context_get_data(ctx);
    ASSERT_EQUAL(first_ptr, second_ptr, "data pointer remains stable after update");
    ASSERT_EQUAL(second_ptr->id, 99, "second update reflects in data");

    inspector_context_destroy(ctx);
}

static void test_context_contains(void)
{
    printf("\n=== test_context_contains ===\n");

    inspector_context_t *ctx = inspector_context_create(&simple_t_info, "obj");
    ASSERT_NOT_NULL(ctx, "inspector_context_create");

    /* After binding, the context should have registered members */
    int has_id = inspector_context_contains(ctx, "obj.id");
    int has_value = inspector_context_contains(ctx, "obj.value");
    ASSERT_TRUE(has_id, "contains(\"obj.id\")");
    ASSERT_TRUE(has_value, "contains(\"obj.value\")");

    int has_invalid = inspector_context_contains(ctx, "obj.invalid");
    ASSERT_EQUAL(has_invalid, 0, "contains(\"obj.invalid\") returns 0");

    inspector_context_destroy(ctx);
}

static void test_context_query_api(void)
{
    printf("\n=== test_context_query_api ===\n");

    inspector_context_t *ctx = inspector_context_create(&simple_t_info, "s");
    ASSERT_NOT_NULL(ctx, "inspector_context_create");

    int size = inspector_context_size(ctx);
    ASSERT_TRUE(size > 0, "inspector_context_size > 0");

    const char *name_0 = inspector_context_name_at(ctx, 0);
    ASSERT_NOT_NULL(name_0, "name_at(0) not NULL");

    const char *type_0 = inspector_context_type_at(ctx, 0);
    ASSERT_NOT_NULL(type_0, "type_at(0) not NULL");

    const char *typename_id = inspector_context_type(ctx, "s.id");
    ASSERT_STR_EQUAL(typename_id, "int", "type(\"s.id\") is \"int\"");

    inspector_context_destroy(ctx);
}

static void test_nested_struct(void)
{
    printf("\n=== test_nested_struct ===\n");

    inspector_context_t *ctx = inspector_context_create(&complex_t_info, "cplx");
    ASSERT_NOT_NULL(ctx, "inspector_context_create");

    complex_t source;
    strcpy(source.name, "sample");
    source.nested.id = 123;
    source.nested.value = 4.56f;

    int ret = inspector_context_update(ctx, &source);
    ASSERT_EQUAL(ret, 0, "update on nested struct");

    const complex_t *owned = (const complex_t *)inspector_context_get_data(ctx);
    ASSERT_EQUAL(owned->nested.id, 123, "nested id copied");
    ASSERT_STR_EQUAL(owned->name, "sample", "name copied");

    inspector_context_destroy(ctx);
}

static void test_null_handling(void)
{
    printf("\n=== test_null_handling ===\n");

    /* Passing NULL should be handled gracefully */
    inspector_context_destroy(NULL);
    printf("PASS: destroy(NULL) doesn't crash\n");
    g_tests_run++;
    g_tests_passed++;

    const void *null_data = inspector_context_get_data(NULL);
    ASSERT_NULL(null_data, "get_data(NULL) returns NULL");

    int null_size = inspector_context_size(NULL);
    ASSERT_EQUAL(null_size, 0, "size(NULL) returns 0");
}

/* ====== Main ====== */

int main(void)
{
    printf("=== inspector context wrapper tests ===\n");

    test_create_destroy();
    test_get_data();
    test_update();
    test_context_contains();
    test_context_query_api();
    test_nested_struct();
    test_null_handling();

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
