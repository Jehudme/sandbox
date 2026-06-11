#pragma once

#include "sandbox/core/ecs.h"
#include <cstdint>
#include <sandbox/api/abi_types.h>

namespace sandbox {

    class irunner {
    public:
        virtual ~irunner() = default;

        virtual int32_t start_async(flecs::world& ecs) = 0;
        virtual int32_t run_sync(flecs::world& ecs) = 0;
        virtual int32_t quit() = 0;
        virtual int32_t pause() = 0;
        virtual int32_t resume() = 0;

        virtual void set_property(const char* key, const char* json_value) = 0;
        virtual int32_t get_property(const char* key, sandbox_payload* out_payload) const = 0;
    };

} // namespace sandbox
