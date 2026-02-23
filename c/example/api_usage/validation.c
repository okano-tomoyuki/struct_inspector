/*!
 * @file api_usage_validation.c
 * @brief Using inspector_context API for runtime type validation and access.
 *
 * Demonstrates the practical value of inspector_context_contains(),
 * inspector_context_type(), inspector_context_get(), and
 * inspector_context_size() APIs for safe, type-aware data access in
 * production code.
 *
 * Real-world scenario: Validating user input against struct schema before
 * processing, without writing manual type-checking code.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Configuration Structure ===== */

typedef struct {
    int port;
    int max_connections;
} network_config_t;

REGISTER_STRUCT_INFO_BEGIN(network_config_t)
    REGISTER_FIELD_SCALAR(network_config_t, port, int, NULL)
    REGISTER_FIELD_SCALAR(network_config_t, max_connections, int, NULL)
REGISTER_STRUCT_INFO_END(network_config_t)

typedef struct {
    char app_name[64];
    char version[32];
    network_config_t network;
    int debug_level;
    float timeout_seconds;
} application_config_t;

REGISTER_STRUCT_INFO_BEGIN(application_config_t)
    REGISTER_FIELD_ARRAY(application_config_t, app_name, char, NULL, 1, 64)
    REGISTER_FIELD_ARRAY(application_config_t, version, char, NULL, 1, 32)
    REGISTER_FIELD_SCALAR(application_config_t, network, network_config_t, &network_config_t_info)
    REGISTER_FIELD_SCALAR(application_config_t, debug_level, int, NULL)
    REGISTER_FIELD_SCALAR(application_config_t, timeout_seconds, float, NULL)
REGISTER_STRUCT_INFO_END(application_config_t)

/* ===== Helper: Validate field exists and has expected type ===== */

typedef struct {
    int valid;
    const char* error;
} validation_result_t;

static validation_result_t validate_field(
    inspector_context_t* ctx,
    const char* field_name,
    const char* expected_type)
{
    validation_result_t result = {.valid = 1, .error = NULL};

    /* Use inspector_context_contains API */
    if (!inspector_context_contains(ctx, field_name)) {
        result.valid = 0;
        result.error = "Field does not exist";
        return result;
    }

    /* Use inspector_context_type API */
    const char* actual_type = inspector_context_type(ctx, field_name);
    if (!actual_type) {
        result.valid = 0;
        result.error = "Cannot determine type";
        return result;
    }

    if (strcmp(actual_type, expected_type) != 0) {
        result.valid = 0;
        result.error = "Type mismatch";
        return result;
    }

    return result;
}

/* ===== Main: Demonstrate API-driven validation ===== */

int main(void)
{
    printf("=== Inspector Context API: Runtime Validation & Type-Safe Access ===\n\n");

    /* Create a context (owns data and internal inspector) */
    inspector_context_t* ctx = inspector_context_create(
        &application_config_t_info,
        "config"
    );

    printf("1. Context created with prefix='config' and owns data\n\n");

    /* Initialize data through context's get_data() */
    const void* data_ptr = inspector_context_get_data(ctx);
    application_config_t* config = (application_config_t*)data_ptr;
    
    strcpy(config->app_name, "MyService");
    strcpy(config->version, "1.0.5");
    config->network.port = 8080;
    config->network.max_connections = 1000;
    config->debug_level = 2;
    config->timeout_seconds = 30.5f;

    printf("2. Data initialized through context:\n");
    printf("   app_name='%s'\n", config->app_name);
    printf("   version='%s'\n", config->version);
    printf("   network.port=%d\n", config->network.port);
    printf("   network.max_connections=%d\n\n", config->network.max_connections);

    printf("3. Validating fields using inspector_context API:\n");

    /* Validation test cases: field_name, expected_type */
    struct {
        const char* field_name;
        const char* expected_type;
    } validations[] = {
        {"config.app_name", "char"},
        {"config.version", "char"},
        {"config.network", "network_config_t"},
        {"config.debug_level", "int"},
        {"config.timeout_seconds", "float"},
        {"config.nonexistent", "int"},
        {"config.debug_level", "float"},  /* Wrong type */
    };

    int num_validations = sizeof(validations) / sizeof(validations[0]);

    for (int i = 0; i < num_validations; i++) {
        const char* field = validations[i].field_name;
        const char* expected = validations[i].expected_type;
        validation_result_t vr = validate_field(ctx, field, expected);

        printf("   Field '%s' (expect type '%s'): %s",
               field, expected,
               vr.valid ? "[o] VALID\n" : "[x] INVALID");
        if (!vr.valid) {
            printf(" [%s]\n", vr.error);
        }
    }

    printf("\n4. Safe field access using inspector_context_get():\n");

    /* Access integer field via context - owns the data */
    void* debug_ptr = inspector_context_get(ctx, "config.debug_level");
    if (debug_ptr) {
        int debug_level = *(int*)debug_ptr;
        printf("   debug_level via context API: %d\n", debug_level);
    }

    /* Access float field */
    void* timeout_ptr = inspector_context_get(ctx, "config.timeout_seconds");
    if (timeout_ptr) {
        float timeout = *(float*)timeout_ptr;
        printf("   timeout_seconds via context API: %.1f\n", timeout);
    }

    printf("\n5. Using inspector_context_contains() for conditional logic:\n");

    struct {
        const char* field;
    } feature_checks[] = {
        {"config.debug_level"},
        {"config.tls_enabled"},
        {"config.network"},
    };

    for (int i = 0; i < 3; i++) {
        const char* field = feature_checks[i].field;
        int has_field = inspector_context_contains(ctx, field);
        printf("   Feature '%s' %s\n",
               field,
               has_field ? "supported [o]" : "not supported [x]");
    }

    printf("\n6. Field count via inspector_context_size():\n");
    int field_count = inspector_context_size(ctx);
    printf("   Total fields in application_config_t: %d\n", field_count);

    printf("\n7. Direct field iteration via size() and type_at():\n");
    for (int i = 0; i < field_count; i++) {
        const char* name = inspector_context_name_at(ctx, i);
        const char* type = inspector_context_type_at(ctx, i);
        printf("   [%d] %s : %s\n", i, name ? name : "?", type ? type : "?");
    }

    printf("\n=== KEY BENEFITS ===\n");
    printf("- inspector_context_contains() - check field existence\n");
    printf("- inspector_context_type() - runtime type verification\n");
    printf("- inspector_context_get() - safe offset-based access\n");
    printf("- inspector_context_size() - determine struct complexity\n");
    printf("- inspector_context_name_at() - iterate by index\n");
    printf("- inspector_context_type_at() - get type by index\n");
    printf("- inspector_context owns data - fixed address lifetime\n");
    printf("\nPractical uses:\n");
    printf("- IPC/RPC deserialization with validation\n");
    printf("- Plugin systems with schema negotiation\n");
    printf("- Configuration file loading with type checking\n");
    printf("- JSON/XML parsing with struct mapping\n");

    /* Cleanup */
    inspector_context_destroy(ctx);

    return 0;
}
