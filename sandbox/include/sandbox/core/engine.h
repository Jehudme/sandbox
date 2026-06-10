#pragma once

#include "sandbox/core/platform.h"

namespace flecs { struct world; }

namespace sandbox {

    class SANDBOX_API engine {
    public:
        engine(const char* json_config);
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept;
        engine& operator=(engine&&) noexcept;

        void run();

        flecs::world& get_ecs();

    private:
        struct impl;
        impl* m_impl;
    };

}