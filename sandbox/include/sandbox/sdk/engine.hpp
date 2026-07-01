#pragma once
#include "sandbox/abi/engine.h"
#include "properties.hpp"

namespace sandbox {
    class properties;
    class engine {
    public:
        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&& other) noexcept;
        engine& operator=(engine&& other) noexcept;

        explicit engine(sandbox_engine_t* raw);

        bool initialize(const properties& props);
        
        // Returns the ECS world managed by the engine (ecs_world_t*)
        ecs_world_t *get_ecs() const;

        sandbox_engine_t* get_raw() const { return m_engine; }
        
    private:
        sandbox_engine_t* m_engine;
    };
}

#include "detail/engine.inl"
