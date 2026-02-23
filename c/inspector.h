/**
 * @file inspector.h
 * @brief Lightweight runtime inspector API for binding and querying object members.
 */

#ifndef INSPECTOR_H
#define INSPECTOR_H

/** Opaque inspector object. Use provided functions to create and destroy. */
typedef struct inspector_t inspector_t;

/**
 * @brief Create a new inspector instance.
 * @return Pointer to a newly allocated inspector_t, or NULL on allocation failure.
 */
inspector_t* inspector_create(void);

/**
 * @brief Destroy an inspector instance and free its resources.
 * @param obj Pointer returned by inspector_create.
 */
void inspector_destroy(inspector_t* obj);

/**
 * @brief Register a named member reference with the inspector.
 * @param obj Inspector instance.
 * @param name Logical name for the member.
 * @param type_name Human-readable type name for diagnostics.
 * @param ref Pointer to the target member (raw pointer into bound object or other storage).
 */
void inspector_add(inspector_t* obj, const char* name, const char* type_name, void* ref);

/**
 * @brief Check whether a named member is present in the inspector.
 * @param obj Inspector instance.
 * @param name Member name to query.
 * @return Non-zero if found, zero otherwise.
 */
int inspector_contains(const inspector_t* obj, const char* name);

/**
 * @brief Get the registered type name for a named member.
 * @param obj Inspector instance.
 * @param name Member name to query.
 * @return Pointer to a null-terminated type name string, or NULL if not found.
 */
const char* inspector_type(const inspector_t* obj, const char* name);

/**
 * @brief Get the pointer previously registered for a named member.
 * @param obj Inspector instance.
 * @param name Member name to query.
 * @return Pointer to the registered object, or NULL if not found.
 */
void* inspector_get(const inspector_t* obj, const char* name);

/**
 * @brief Number of registered members.
 * @param obj Inspector instance.
 * @return Count of registered members, or 0 for NULL obj.
 */
int inspector_size(const inspector_t* obj);

/**
 * @brief Get the name of the member at the given index.
 * @param obj Inspector instance.
 * @param index Zero-based index of the member.
 * @return Pointer to the name string, or NULL if index is out of range.
 */
const char* inspector_name_at(const inspector_t* obj, int index);

/**
 * @brief Get the type name of the member at the given index.
 * @param obj Inspector instance.
 * @param index Zero-based index of the member.
 * @return Pointer to the type name string, or NULL if index is out of range.
 */
const char* inspector_type_at(const inspector_t* obj, int index);

#endif // INSPECTOR_H