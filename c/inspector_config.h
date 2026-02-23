/*!
 * @file inspector_config.h
 * @brief Central configuration macros for the struct inspector library.
 *
 * This header contains compile-time constants that control buffer sizes and
 * DSL symbol naming. Keep only configuration values here; feature switches
 * should be controlled by the build system where possible.
 */

#ifndef INSPECTOR_CONFIG_H
#define INSPECTOR_CONFIG_H

/** Maximum length for human-readable names used by the inspector. */
#define INSPECTOR_NAME_MAX             128

/** Maximum length for type name strings. */
#define INSPECTOR_TYPE_MAX             64

/** Maximum path length used for member path representations. */
#define INSPECTOR_PATH_MAX             512

/** Postfix appended to generated struct info symbols (default: _info). */
#define INSPECTOR_STRUCT_INFO_POSTFIX  _info

#endif