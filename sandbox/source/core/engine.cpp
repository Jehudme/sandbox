#include "engine.h"

#include <flecs/addons/cpp/flecs.hpp>

#include "bootstrapper.h"
#include "sandbox/core/argument.h"

namespace sandbox::core {
    engine_t::engine_t() = default;
    engine_t::~engine_t() = default;

    void engine_t::initialize(properties_t& properties) {
        ecs.reset();
        
        m_arguments = std::make_unique<properties_t>(properties);
        
        ECS_COMPONENT_DEFINE(ecs.c_ptr(), sandbox_argument_t);
        
        sandbox_argument_t arg_comp;
        arg_comp.internal_properties = reinterpret_cast<sandbox_properties_t*>(m_arguments.get());
        
        ecs_set_id(ecs.c_ptr(), ecs_id(sandbox_argument_t), ecs_id(sandbox_argument_t), sizeof(sandbox_argument_t), &arg_comp);
    }
}
