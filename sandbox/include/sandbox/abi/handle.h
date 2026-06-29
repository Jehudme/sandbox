#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generates a strictly typed ABI handle.
 * By wrapping the generic integer token in a uniquely named struct,
 * the C/C++ compiler will throw a hard error if an external user
 * accidentally passes the wrong handle type to a function.
 */
#define SANDBOX_DEFINE_HANDLE(name) typedef struct { uintptr_t token; } name

/**
 * @brief Standard status codes for operations that modify data.
 * Note: Functions that *create* objects should return the handle directly.
 */
typedef int32_t sandbox_status_t;
#define SANDBOX_STATUS_SUCCESS          0
#define SANDBOX_STATUS_ERROR_INVALID    -1
#define SANDBOX_STATUS_ERROR_NULL_PTR   -2
#define SANDBOX_STATUS_ERROR_INTERNAL   -3

/**
 * @brief Helper macro to check if a handle is valid (not zeroed out).
 */
#define SANDBOX_HANDLE_IS_VALID(handle) ((handle).token != 0)

#ifdef __cplusplus
} /* extern "C" */
#endif