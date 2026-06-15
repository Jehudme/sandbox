#include "engine.h"

#include <flecs/addons/cpp/flecs.hpp>

#include "bootstrapper.h"

namespace sandbox::core {
    engine_t::engine_t() {
    }

    engine_t::~engine_t() {
    }

    void engine_t::initialize(properties_t& properties) {
        ecs.reset();

        auto arguments_entt = ecs.entity("::Sandbox::Arguments")
            .set<properties_t>(properties);

        bootstrapper_t bootstrapper;

    }
}
