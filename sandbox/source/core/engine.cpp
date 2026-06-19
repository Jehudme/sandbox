#include "engine.h"

#include <flecs/addons/cpp/flecs.hpp>

#include "bootstrapper.h"
#include "sandbox/core/argument.h"
#include "sandbox/core/bootstrapper.h"
#include "client_parser.h"

namespace sandbox::core {
    engine_t::engine_t() = default;
    engine_t::~engine_t() = default;

    void engine_t::initialize(properties_t& properties) {
        ecs.reset();
        
        m_arguments = std::make_unique<properties_t>(properties);
        m_bootstrapper = std::make_unique<bootstrapper_t>();

        save_arguments();
        save_bootstrapper();

        boot();
    }

    void engine_t::save_arguments() {
        ECS_COMPONENT_DEFINE(ecs.c_ptr(), sandbox_argument_t);
        sandbox_argument_t arg_comp;
        arg_comp.internal_properties = reinterpret_cast<sandbox_properties_t*>(m_arguments.get());
        ecs_set_id(ecs.c_ptr(), ecs_id(sandbox_argument_t), ecs_id(sandbox_argument_t), sizeof(sandbox_argument_t), &arg_comp);
    }

    void engine_t::save_bootstrapper() {
        ECS_COMPONENT_DEFINE(ecs.c_ptr(), sandbox_bootstrapper_component_t);
        sandbox_bootstrapper_component_t boot_comp;
        boot_comp.internal_bootstrapper = reinterpret_cast<sandbox_bootstrapper_t*>(m_bootstrapper.get());
        ecs_set_id(ecs.c_ptr(), ecs_id(sandbox_bootstrapper_component_t), ecs_id(sandbox_bootstrapper_component_t), sizeof(sandbox_bootstrapper_component_t), &boot_comp);
    }

    void engine_t::boot() {
        client_parser_t::apply_configuration(*m_bootstrapper, *m_arguments);
        m_bootstrapper->boot(ecs);
    }
}
