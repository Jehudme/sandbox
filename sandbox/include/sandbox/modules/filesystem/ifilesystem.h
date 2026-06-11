#pragma once

#include "sandbox/core/service_macro.h"

namespace sandbox {
    DECLARE_SANDBOX_SERVICE(filesystem_service, "filesystem_service", 1, 0)

    class ifilesystem {
    public:
        virtual ~ifilesystem() = default;

        [[nodiscard]] virtual int32_t mount(const char* physical_path, const char* virtual_prefix, bool read_only) = 0;
        [[nodiscard]] virtual int32_t unmount(const char* virtual_prefix) = 0;
        [[nodiscard]] virtual int32_t read(const char* virtual_path, sandbox_payload* out_payload) const = 0;
        [[nodiscard]] virtual int32_t write(const char* virtual_path, const uint8_t* data, size_t size, bool append) = 0;
        [[nodiscard]] virtual int32_t list(const char* virtual_path, bool recursive, sandbox_payload* out_payload) const = 0;
        [[nodiscard]] virtual int32_t remove(const char* virtual_path) = 0;
        [[nodiscard]] virtual int32_t mkdir(const char* virtual_path) = 0;
        [[nodiscard]] virtual int32_t rename(const char* old_virtual_path, const char* new_virtual_path) = 0;
        [[nodiscard]] virtual int32_t copy(const char* source_virtual_path, const char* destination_virtual_path) = 0;
        [[nodiscard]] virtual int32_t move(const char* source_virtual_path, const char* destination_virtual_path) = 0;
        [[nodiscard]] virtual int32_t state(const char* virtual_path, sandbox_payload* out_payload) const = 0;
        [[nodiscard]] virtual int32_t absolute(const char* virtual_path, sandbox_payload* out_payload) const = 0;

        virtual void set_property(const char* key, const char* json_value) = 0;
        virtual int32_t get_property(const char* key, sandbox_payload* out_payload) const = 0;
    };

} // namespace sandbox
