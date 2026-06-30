#pragma once
#include "flecs/addons/cpp/world.hpp"

namespace sandbox::launcher {
    class application_t {
    public:
        application_t(flecs::world& ecs);
        ~application_t();

        application_t(const application_t&) = delete;
        application_t& operator=(const application_t&) = delete;
    };
}
