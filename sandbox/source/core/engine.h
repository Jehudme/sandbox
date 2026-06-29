#pragma once
#include "properties.h"
#include <flecs.h>
#include <memory>

namespace sandbox::core {
    class bootstrapper_t;
    class engine_t {
    public:
        engine_t();
        ~engine_t();

        void initialize(properties_t& properties);

    public:
        flecs::world ecs;

    private:
        void save_bootstrapper();

        void boot();


    private:
        std::unique_ptr<properties_t> m_arguments;
        std::unique_ptr<bootstrapper_t> m_bootstrapper;
    };
}
