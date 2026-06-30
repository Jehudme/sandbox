#pragma once

#include <flecs.h>

namespace sandbox::modules {
    struct configuration_module_t {
        explicit configuration_module_t(flecs::world& world);
    };
}
