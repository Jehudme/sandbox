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

#ifdef __cplusplus
#define DECLARE_SANDBOX_SERVICE(ServiceName, ServiceStringName, MajorVer, MinorVer) \
struct ServiceName { \
    void* instance; \
    void (*execute_command)(void* instance, uint32_t command_id, const uint8_t* payload, size_t size); \
    static constexpr const char* service_name = ServiceStringName; \
    static constexpr uint8_t service_major = MajorVer; \
    static constexpr uint8_t service_minor = MinorVer; \
};
#else
#define DECLARE_SANDBOX_SERVICE(ServiceName, ServiceStringName, MajorVer, MinorVer) \
struct ServiceName { \
    void* instance; \
    void (*execute_command)(void* instance, uint32_t command_id, const uint8_t* payload, size_t size); \
};
#endif

#ifdef __cplusplus
}

namespace sandbox::detail {
    template<typename T> struct service_traits {
        static constexpr const char* name = T::service_name;
        static constexpr uint8_t major = T::service_major;
        static constexpr uint8_t minor = T::service_minor;
    };
}
#endif
