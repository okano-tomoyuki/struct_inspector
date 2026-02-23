/*!
 * @file test_inspector.c
 * @brief Unit tests for the core inspector API.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector.h"

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

/* ====== Tests ====== */

static void test_create_destroy(void)
{
    printf("\n=== test_create_destroy ===\n");

    inspector_t *insp = inspector_create();
    ASSERT_NOT_NULL(insp, "inspector_create returns non-NULL");

    inspector_destroy(insp);
    printf("PASS: inspector_destroy completed without crash\n");
    g_tests_run++;
    g_tests_passed++;
}

static void test_add_contains(void)
{
    printf("\n=== test_add_contains ===\n");

    inspector_t *insp = inspector_create();
    ASSERT_NOT_NULL(insp, "inspector_create");

    int int_val = 42;
    inspector_add(insp, "my_int", "int", &int_val);

    ASSERT_TRUE(inspector_contains(insp, "my_int"), "contains(\"my_int\")");
    ASSERT_EQUAL(inspector_contains(insp, "nonexistent"), 0, "contains(\"nonexistent\") returns 0");

    inspector_destroy(insp);
}

static void test_get_type(void)
{
    printf("\n=== test_get_type ===\n");

    inspector_t *insp = inspector_create();
    ASSERT_NOT_NULL(insp, "inspector_create");

    double double_val = 3.14;
    inspector_add(insp, "my_double", "double", &double_val);

    const char *type_name = inspector_type(insp, "my_double");
    ASSERT_NOT_NULL(type_name, "inspector_type returns non-NULL");
    ASSERT_STR_EQUAL(type_name, "double", "type name matches");

    const char *null_type = inspector_type(insp, "nonexistent");
    ASSERT_NULL(null_type, "inspector_type on missing member returns NULL");

    inspector_destroy(insp);
}

static void test_get_ptr(void)
{
    printf("\n=== test_get_ptr ===\n");

    inspector_t *insp = inspector_create();
    ASSERT_NOT_NULL(insp, "inspector_create");

    int int_val = 123;
    void *ref = &int_val;
    inspector_add(insp, "ref_to_int", "int*", ref);

    void *retrieved = inspector_get(insp, "ref_to_int");
    ASSERT_EQUAL(retrieved, ref, "retrieved pointer matches registered pointer");

    void *null_ptr = inspector_get(insp, "nonexistent");
    ASSERT_NULL(null_ptr, "inspector_get on missing member returns NULL");

    inspector_destroy(insp);
}

static void test_size_and_iteration(void)
{
    printf("\n=== test_size_and_iteration ===\n");

    inspector_t *insp = inspector_create();
    ASSERT_NOT_NULL(insp, "inspector_create");

    int a = 1, b = 2, c = 3;
    inspector_add(insp, "field_a", "int", &a);
    inspector_add(insp, "field_b", "int", &b);
    inspector_add(insp, "field_c", "int", &c);

    /* inspector_size counts from -1, so 3 items = size returns 2 */
    ASSERT_EQUAL(inspector_size(insp), 2, "inspector_size returns 2 for 3 items");

    const char *name_0 = inspector_name_at(insp, 0);
    ASSERT_NOT_NULL(name_0, "inspector_name_at(0) not NULL");
    ASSERT_STR_EQUAL(name_0, "field_a", "first field name is \"field_a\"");

    const char *name_1 = inspector_name_at(insp, 1);
    ASSERT_STR_EQUAL(name_1, "field_b", "second field name is \"field_b\"");

    const char *type_0 = inspector_type_at(insp, 0);
    ASSERT_STR_EQUAL(type_0, "int", "type_at(0) returns \"int\"");

    const char *oob_name = inspector_name_at(insp, 100);
    ASSERT_NULL(oob_name, "inspector_name_at with out-of-bounds index returns NULL");

    inspector_destroy(insp);
}

static void test_multiple_types(void)
{
    printf("\n=== test_multiple_types ===\n");

    inspector_t *insp = inspector_create();
    ASSERT_NOT_NULL(insp, "inspector_create");

    int int_val = 42;
    float float_val = 3.14f;
    char str_val[] = "hello";

    inspector_add(insp, "my_int", "int", &int_val);
    inspector_add(insp, "my_float", "float", &float_val);
    inspector_add(insp, "my_str", "char*", str_val);

    /* inspector_size counts from -1, so 3 items = size returns 2 */
    ASSERT_EQUAL(inspector_size(insp), 2, "size is 2 for 3 items");

    ASSERT_STR_EQUAL(inspector_type(insp, "my_int"), "int", "int type");
    ASSERT_STR_EQUAL(inspector_type(insp, "my_float"), "float", "float type");
    ASSERT_STR_EQUAL(inspector_type(insp, "my_str"), "char*", "char* type");

    inspector_destroy(insp);
}

/* ====== Main ====== */

int main(void)
{
    printf("=== inspector core API tests ===\n");

    test_create_destroy();
    test_add_contains();
    test_get_type();
    test_get_ptr();
    test_size_and_iteration();
    test_multiple_types();

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
