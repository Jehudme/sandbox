#pragma once

#include "sandbox/core/ecs.h"

namespace sandbox {

    class irunner {
    public:
        virtual ~irunner() = default;

        virtual void start_async(flecs::world& ecs) = 0;
        virtual void run_sync(flecs::world& ecs) = 0;
        virtual void quit() = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
    };

    struct runner_service {
        irunner* api{nullptr};
    };

} // namespace sandbox
