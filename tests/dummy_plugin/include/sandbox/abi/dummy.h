#pragma once
#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

// The API for the dummy service
typedef struct {
    int (*get_magic_number)(void);
} sandbox_dummy_api_t;

// Declare the external API instance that will be implemented in library.cpp
SANDBOX_DECLARE_SERVICE(sandbox_dummy_service_t, sandbox_dummy_api_t, {
    .name = "dummy",
    .description = "The dummy service",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0
})

#ifdef __cplusplus
}
#endif
