// modules/configuration/include/sandbox/abi/configuration.h
#pragma once
#include <sandbox/abi/bootstrapper.h>
#include <sandbox/abi/properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// The API for the configuration service
typedef struct {
    sandbox_properties_handle_t (*get_properties)(void);
} sandbox_configuration_api_t;

SANDBOX_DECLARE_SERVICE(sandbox_configuration_service_t, sandbox_configuration_api_t)

#ifdef __cplusplus
}
#endif
