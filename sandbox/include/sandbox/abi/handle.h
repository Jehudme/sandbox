#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration of the hidden internal object */
typedef struct sandbox_opaque_object sandbox_opaque_object_t;

/* The ABI-safe token */
typedef struct {
    uintptr_t token;
} sandbox_handle_t;

/* ABI status return codes */
typedef int32_t sandbox_status_t;
#define SANDBOX_STATUS_SUCCESS          0
#define SANDBOX_STATUS_ERROR_INVALID    -1
#define SANDBOX_STATUS_ERROR_NULL_PTR   -2

/* Macro to check if a handle is zeroed out */
#define SANDBOX_HANDLE_IS_INVALID(h) ((h).token == 0)

/* Exported ABI Functions */
sandbox_status_t sandbox_handle_create(sandbox_handle_t* out_handle);
sandbox_status_t sandbox_handle_destroy(sandbox_handle_t* handle);
bool sandbox_handle_is_valid(sandbox_handle_t handle);

#ifdef __cplusplus
} /* extern "C" */
#endif