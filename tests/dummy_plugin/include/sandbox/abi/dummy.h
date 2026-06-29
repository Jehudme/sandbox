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
extern sandbox_dummy_api_t g_dummy_api;

// The service is declared publicly here, so any other module including this header 
// can fetch it and interact with its API.
SANDBOX_DECLARE_SERVICE(sandbox_dummy_service_t, sandbox_dummy_api_t, &g_dummy_api, {
    .name = "dummy",
    .description = "A professional dummy service",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0
})

#ifdef __cplusplus
}
#endif
