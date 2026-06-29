#include "sandbox/abi/handle.h"

// Define the actual internal data structure here.
// Completely hidden from the ABI header.
struct sandbox_opaque_object {
    int32_t internal_id;
    // Add anything you need here
};

sandbox_status_t sandbox_handle_create(sandbox_handle_t* out_handle) {
    if (!out_handle) {
        return SANDBOX_STATUS_ERROR_NULL_PTR;
    }

    auto* internal_obj = new sandbox_opaque_object();
    internal_obj->internal_id = 42;

    // Raw cast at the ABI boundary
    out_handle->token = reinterpret_cast<uintptr_t>(internal_obj);

    return SANDBOX_STATUS_SUCCESS;
}

sandbox_status_t sandbox_handle_destroy(sandbox_handle_t* handle) {
    if (!handle || SANDBOX_HANDLE_IS_INVALID(*handle)) {
        return SANDBOX_STATUS_ERROR_INVALID;
    }

    // Raw unpack and destroy
    auto* internal_obj = reinterpret_cast<sandbox_opaque_object*>(handle->token);
    delete internal_obj;

    handle->token = 0;
    return SANDBOX_STATUS_SUCCESS;
}

bool sandbox_handle_is_valid(sandbox_handle_t handle) {
    return !SANDBOX_HANDLE_IS_INVALID(handle);
}