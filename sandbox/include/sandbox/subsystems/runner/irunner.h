#pragma once

#include "sandbox/core/ecs.h"
#include <string>
#include <any>

namespace sandbox {

    class irunner {
    public:
        virtual ~irunner() = default;

        virtual void start_async(flecs::world& ecs) = 0;
        virtual void run_sync(flecs::world& ecs) = 0;
        virtual void quit() = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;

        virtual void set_property(const std::string& key, const std::any& value) = 0;
        virtual std::any get_property(const std::string& key) const = 0;
    };

    struct runner_service {
        irunner* api{nullptr};
    };

} // namespace sandbox
