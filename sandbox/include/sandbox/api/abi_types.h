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

namespace flecs { struct world; }

namespace sandbox {
    struct logger_service {
        void* instance;
        int32_t (*log)(void* instance, const uint8_t* log_msg_fb, size_t size);
        void (*set_property)(void* instance, const char* key, const char* json_value);
        int32_t (*get_property)(const void* instance, const char* key, sandbox_payload* out_payload);
    };

    struct runner_service {
        void* instance;
        int32_t (*start_async)(void* instance, flecs::world& ecs);
        int32_t (*run_sync)(void* instance, flecs::world& ecs);
        int32_t (*quit)(void* instance);
        int32_t (*pause)(void* instance);
        int32_t (*resume)(void* instance);
        void (*set_property)(void* instance, const char* key, const char* json_value);
        int32_t (*get_property)(const void* instance, const char* key, sandbox_payload* out_payload);
    };

    struct filesystem_service {
        void* instance;
        int32_t (*mount)(void* instance, const char* physical_path, const char* virtual_prefix, bool read_only);
        int32_t (*unmount)(void* instance, const char* virtual_prefix);
        int32_t (*read)(const void* instance, const char* virtual_path, sandbox_payload* out_payload);
        int32_t (*write)(void* instance, const char* virtual_path, const uint8_t* data, size_t size, bool append);
        int32_t (*list)(const void* instance, const char* virtual_path, bool recursive, sandbox_payload* out_payload);
        int32_t (*remove)(void* instance, const char* virtual_path);
        int32_t (*mkdir)(void* instance, const char* virtual_path);
        int32_t (*rename)(void* instance, const char* old_virtual_path, const char* new_virtual_path);
        int32_t (*copy)(void* instance, const char* source_virtual_path, const char* destination_virtual_path);
        int32_t (*move)(void* instance, const char* source_virtual_path, const char* destination_virtual_path);
        int32_t (*state)(const void* instance, const char* virtual_path, sandbox_payload* out_payload);
        int32_t (*absolute)(const void* instance, const char* virtual_path, sandbox_payload* out_payload);
        void (*set_property)(void* instance, const char* key, const char* json_value);
        int32_t (*get_property)(const void* instance, const char* key, sandbox_payload* out_payload);
    };

    namespace abi {
        using flatbuffer_payload = sandbox_payload;
    }
}
#endif
