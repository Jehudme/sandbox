#pragma once
#include "../engine.hpp"

namespace sandbox {

    inline engine::engine() {
        m_engine = sandbox_engine_create();
    }

    inline engine::~engine() {
        sandbox_engine_destroy(m_engine);
    }

    inline engine::engine(engine&& other) noexcept : m_engine(other.m_engine) {
        other.m_engine = nullptr;
    }

    inline engine& engine::operator=(engine&& other) noexcept {
        if (this != &other) {
            sandbox_engine_destroy(m_engine);
            m_engine = other.m_engine;
            other.m_engine = nullptr;
        }
        return *this;
    }

    inline engine::engine(sandbox_engine_t* raw) : m_engine(raw) {}

    inline bool engine::initialize(const properties& props) {
        return sandbox_engine_initialize(m_engine, props.get_raw());
    }

    inline void* engine::get_ecs() const {
        return sandbox_engine_get_ecs(m_engine);
    }

}
