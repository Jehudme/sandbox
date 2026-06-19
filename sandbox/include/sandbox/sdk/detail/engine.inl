#pragma once
#include "../engine.hpp"

namespace sandbox {

    inline Engine::Engine() {
        m_engine = sandbox_engine_create();
    }

    inline Engine::~Engine() {
        sandbox_engine_destroy(m_engine);
    }

    inline Engine::Engine(Engine&& other) noexcept : m_engine(other.m_engine) {
        other.m_engine = nullptr;
    }

    inline Engine& Engine::operator=(Engine&& other) noexcept {
        if (this != &other) {
            sandbox_engine_destroy(m_engine);
            m_engine = other.m_engine;
            other.m_engine = nullptr;
        }
        return *this;
    }

    inline Engine::Engine(sandbox_engine_t* raw) : m_engine(raw) {}

    inline bool Engine::initialize(const Properties& properties) {
        return sandbox_engine_initialize(m_engine, properties.get_raw());
    }

    inline void* Engine::get_ecs() const {
        return sandbox_engine_get_ecs(m_engine);
    }

}
