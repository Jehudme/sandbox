#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sandbox_payload {
    uint8_t* bytes;
    size_t size;
    void (*free_func)(void*);
} sandbox_payload;

#ifdef __cplusplus
}
#endif
