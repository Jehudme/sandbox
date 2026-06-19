#pragma once
#include "properties.h"
#include <flecs.h>

namespace sandbox::core {
    class engine_t {
    public:
        engine_t();
        ~engine_t();

        void initialize(properties_t& properties);

    public:
        flecs::world ecs;

    private:
        std::unique_ptr<properties_t> m_arguments;
    };
}
