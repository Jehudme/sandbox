#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sandbox_payload {
    void* bytes;
    size_t size;
    void (*free_func)(void*);
} sandbox_payload;

#define DECLARE_SANDBOX_SERVICE(ServiceName) \
struct ServiceName { \
    void* instance; \
    void (*execute_command)(void* instance, uint32_t command_id, const uint8_t* payload, size_t size); \
};

#ifdef __cplusplus
}
#endif
