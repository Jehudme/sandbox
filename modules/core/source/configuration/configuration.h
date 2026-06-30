#pragma once

#include <flecs.h>

namespace sandbox::modules {
    /**
     * @brief A global module that manages the engine's dynamic properties.
     */
    struct configuration_module_t {
        /**
         * @brief Initializes the configuration module.
         * @param entity_world The flecs world.
         */
        explicit configuration_module_t(flecs::world& entity_world);
    };
}
