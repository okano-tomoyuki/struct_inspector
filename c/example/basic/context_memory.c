/*!
 * @file context_memory_example.c
 * @brief Example demonstrating safe memory management via inspector_context.
 *
 * This example showcases the key advantages of inspector_context:
 * - Fixed address lifetime (memcpy-based updates, no reallocation)
 * - Data ownership (context owns memory, not external references)
 * - No dangling pointers after context destruction
 * - Predictable memory model for embedded systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inspector_context.h"

#define ENABLE_INSPECTOR_BIND_DSL
#include "inspector_dsl.h"

/* ===== Simple data structure ===== */

typedef struct
{
    int counter;
    float state[4];
    char status[32];
} device_state_t;

REGISTER_STRUCT_INFO_BEGIN(device_state_t)
REGISTER_FIELD_SCALAR(device_state_t, counter, int, NULL)
REGISTER_FIELD_ARRAY(device_state_t, state, float, NULL, 1, 4)
REGISTER_FIELD_ARRAY(device_state_t, status, char, NULL, 1, 32)
REGISTER_STRUCT_INFO_END(device_state_t)

/* ===== Simulation of device operations ===== */

typedef struct
{
    inspector_context_t *state_context;
} device_t;

static device_t *device_create(void)
{
    device_t *dev = (device_t *)malloc(sizeof(device_t));
    if (!dev)
        return NULL;

    dev->state_context = inspector_context_create(&device_state_t_info, "device");
    if (!dev->state_context)
    {
        free(dev);
        return NULL;
    }

    return dev;
}

static void device_destroy(device_t *dev)
{
    if (!dev)
        return;
    if (dev->state_context)
    {
        inspector_context_destroy(dev->state_context);
    }
    free(dev);
}

static device_state_t *device_get_state(device_t *dev)
{
    if (!dev)
        return NULL;
    return (device_state_t *)inspector_context_get_data(dev->state_context);
}

static void device_update_state(device_t *dev, const device_state_t *new_state)
{
    if (!dev)
        return;
    inspector_context_update(dev->state_context, new_state);
}

/* ===== Main demonstration ===== */

int main(void)
{
    printf("=== Context Memory Safety Example ===\n\n");

    /* Create a simulated device */
    device_t *device = device_create();
    if (!device)
    {
        fprintf(stderr, "Error: Failed to create device\n");
        return 1;
    }

    printf("Device created with managed state.\n\n");

    /* Initialize state */
    device_state_t initial_state = {
        .counter = 0,
        .state = {0.0f, 0.0f, 0.0f, 0.0f},
        .status = "INIT"};
    device_update_state(device, &initial_state);

    /* Get stable pointer to device state */
    device_state_t *state = device_get_state(device);
    const device_state_t *state_ptr_saved = state; /* Save pointer */

    printf("Initial state retained at: %p\n", (void *)state);
    printf("  Counter: %d\n", state->counter);
    printf("  Status: %s\n\n", state->status);

    /* Simulate device operations: multiple updates */
    printf("--- Performing device operations ---\n");

    for (int i = 1; i <= 3; i++)
    {
        /* Create new state data (external source) */
        device_state_t new_state;
        new_state.counter = i;
        new_state.state[0] = i * 1.5f;
        new_state.state[1] = i * 2.5f;
        new_state.state[2] = i * 3.5f;
        new_state.state[3] = i * 4.5f;
        sprintf(new_state.status, "RUNNING_%d", i);

        /* Update device state */
        device_update_state(device, &new_state);

        /* Get state pointer again */
        state = device_get_state(device);

        printf("Operation %d:\n", i);
        printf("  State address: %p (fixed: %s)\n",
               (void *)state,
               (state == state_ptr_saved) ? "YES" : "NO");
        printf("  Counter: %d\n", state->counter);
        printf("  State[0]: %.2f\n", state->state[0]);
        printf("  Status: %s\n\n", state->status);
    }

    /* ===== Demonstrate key memory safety property ===== */

    printf("--- Memory Safety Verification ---\n");
    printf("Pointer stability check:\n");
    printf("  Initial pointer: %p\n", (void *)state_ptr_saved);

    state = device_get_state(device);
    printf("  Final pointer:   %p\n", (void *)state);
    printf("  Same? %s\n\n", (state == state_ptr_saved) ? "YES ✓" : "NO ✗");

    if (state == state_ptr_saved)
    {
        printf("SUCCESS: Address remained stable throughout updates.\n");
        printf("This guarantees no dangling pointers or reallocation surprises.\n\n");
    }

    /* ===== Cleanup ===== */
    printf("--- Cleanup ---\n");
    device_destroy(device);
    printf("Device destroyed. Owned memory automatically freed.\n");
    printf("It is now UNSAFE to access state pointers.\n");

    return 0;
}
