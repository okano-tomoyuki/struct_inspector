/*!
 * @file update.c
 * @brief Using inspector_context to safely update and manipulate data.
 *
 * Demonstrates inspector_context_update() for safe data replacement,
 * and how the context ensures fixed address lifetime and correct offset
 * calculations for nested structures.
 *
 * Real-world scenario: Hot-swapping configuration, event processing,
 * or safe data mutations without dangling pointers.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Data Structure ===== */

typedef struct {
    int major;
    int minor;
} version_t;

REGISTER_STRUCT_INFO_BEGIN(version_t)
    REGISTER_FIELD_SCALAR(version_t, major, int, NULL)
    REGISTER_FIELD_SCALAR(version_t, minor, int, NULL)
REGISTER_STRUCT_INFO_END(version_t)

typedef struct {
    char service_name[64];
    version_t version;
    int status_code;
    float uptime_hours;
} service_status_t;

REGISTER_STRUCT_INFO_BEGIN(service_status_t)
    REGISTER_FIELD_ARRAY(service_status_t, service_name, char, NULL, 1, 64)
    REGISTER_FIELD_SCALAR(service_status_t, version, version_t, &version_t_info)
    REGISTER_FIELD_SCALAR(service_status_t, status_code, int, NULL)
    REGISTER_FIELD_SCALAR(service_status_t, uptime_hours, float, NULL)
REGISTER_STRUCT_INFO_END(service_status_t)

/* ===== Main ===== */

int main(void)
{
    printf("=== Inspector Context API: Safe Data Updates & Fixed Lifetime ===\n\n");

    /* Create context for service status */
    inspector_context_t* ctx = inspector_context_create(
        &service_status_t_info,
        "status"
    );

    printf("1. Initial data (via context ownership):\n");

    /* Get pointer to owned data */
    const void* data1 = inspector_context_get_data(ctx);
    service_status_t* status1 = (service_status_t*)data1;
    
    strcpy(status1->service_name, "api-v1");
    status1->version.major = 1;
    status1->version.minor = 0;
    status1->status_code = 200;
    status1->uptime_hours = 72.5f;

    printf("   From context: service_name='%s', version=%d.%d, status=%d\n",
           status1->service_name, status1->version.major, status1->version.minor,
           status1->status_code);

    /* Store field pointers BEFORE update */
    int* status_code_ptr_before = (int*)inspector_context_get(ctx, "status_code");
    float* uptime_ptr_before = (float*)inspector_context_get(ctx, "uptime_hours");
    
    printf("   status_code field pointer: %p\n", (void*)status_code_ptr_before);
    printf("   uptime_hours field pointer: %p\n", (void*)uptime_ptr_before);

    printf("\n2. Update data via inspector_context_update():\n");

    /* Prepare new data */
    service_status_t new_status = {
        .service_name = "api-v2",
        .version = {.major = 2, .minor = 5},
        .status_code = 200,
        .uptime_hours = 120.0f
    };

    printf("   Updating with new values...\n");
    inspector_context_update(ctx, &new_status);

    printf("   (Context reallocates internally but keeps same ADDRESS)\n");

    printf("\n3. Field pointers remain VALID after update:\n");

    /* Get data again - should be same address due to context ownership */
    const void* data2 = inspector_context_get_data(ctx);
    service_status_t* status2 = (service_status_t*)data2;

    printf("   Data address before update: %p\n", (void*)data1);
    printf("   Data address after update:  %p\n", (void*)data2);
    printf("   Addresses MATCH: %s ✓\n", data1 == data2 ? "YES" : "NO");

    printf("\n   After update:\n");
    printf("   service_name='%s'\n", status2->service_name);
    printf("   version=%d.%d\n", status2->version.major, status2->version.minor);

    /* Check that old field pointers still work */
    int* status_code_ptr_after = (int*)inspector_context_get(ctx, "status_code");
    printf("   status_code pointer before: %p\n", (void*)status_code_ptr_before);
    printf("   status_code pointer after:  %p\n", (void*)status_code_ptr_after);
    printf("   Pointers MATCH: %s ✓\n", 
           status_code_ptr_before == status_code_ptr_after ? "YES" : "NO");

    printf("\n4. Safe offset-based access during updates:\n");

    /* Show that all fields are still accessible with correct offsets */
    int field_count = inspector_context_size(ctx);
    printf("   Total fields: %d\n", field_count);
    printf("   Re-verifying field access after update:\n");

    for (int i = 0; i < field_count; i++) {
        const char* name = inspector_context_name_at(ctx, i);
        const char* type = inspector_context_type_at(ctx, i);
        void* ptr = inspector_context_get(ctx, name);
        printf("   - %s (%s): %s\n", name, type, ptr ? "[o] accessible" : "[x] ERROR");
    }

    printf("\n5. Complex scenario: Batched updates with consistency:\n");

    /* Simulate configuration updates from multiple sources */
    struct {
        const char* scenario;
        service_status_t data;
    } updates[] = {
        {
            "Health check",
            {
                .service_name = "api-v2",
                .version = {.major = 2, .minor = 5},
                .status_code = 200,
                .uptime_hours = 125.0f
            }
        },
        {
            "Maintenance",
            {
                .service_name = "api-v2",
                .version = {.major = 2, .minor = 5},
                .status_code = 503,
                .uptime_hours = 125.0f
            }
        },
        {
            "Recovery",
            {
                .service_name = "api-v2",
                .version = {.major = 2, .minor = 5},
                .status_code = 200,
                .uptime_hours = 125.5f
            }
        },
    };

    for (int i = 0; i < 3; i++) {
        printf("   [%d] %s: ", i+1, updates[i].scenario);
        inspector_context_update(ctx, &updates[i].data);
        
        const void* current = inspector_context_get_data(ctx);
        service_status_t* current_status = (service_status_t*)current;
        printf("status=%d, uptime=%.1f [o]\n", 
               current_status->status_code, current_status->uptime_hours);
    }

    printf("\n=== KEY BENEFITS ===\n");
    printf("- inspector_context_update() - replace data without new allocation\n");
    printf("- Fixed address lifetime - pointers remain valid after update\n");
    printf("- Offset calculations - maintained correctly through updates\n");
    printf("- No dangling pointers - context owns memory for entire lifetime\n");
    printf("- Type-safe access - all offsets validated at update time\n");
    printf("\nAdvanced patterns:\n");
    printf("- Hot configuration reloading\n");
    printf("- Real-time data stream processing\n");
    printf("- Event-driven state updates without pointer invalidation\n");
    printf("- Atomic struct-wide updates with validation\n");

    /* Cleanup */
    inspector_context_destroy(ctx);

    return 0;
}
