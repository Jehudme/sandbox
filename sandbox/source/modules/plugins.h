#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/events/plugins.h"

namespace sandbox::modules {

    class plugins {
    public:
        plugins(world& ecs);
        ~plugins();

    private:
        void on_load(world& ecs, const events::plugins::load_request& e);
    };

} // namespace sandbox::modules
