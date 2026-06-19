#pragma once
#include "sandbox/abi/engine.h"
#include "properties.hpp"

namespace sandbox {
    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        Engine(Engine&& other) noexcept;
        Engine& operator=(Engine&& other) noexcept;

        explicit Engine(sandbox_engine_t* raw);

        bool initialize(const Properties& properties);
        
        // Returns the ECS world managed by the engine (ecs_world_t*)
        void* get_ecs() const;

        sandbox_engine_t* get_raw() const { return m_engine; }
        
    private:
        sandbox_engine_t* m_engine;
    };
}

#include "detail/engine.inl"
