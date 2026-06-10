#pragma once
#include <stdexcept>

namespace sandbox {

    template<typename TDerived, typename iservice_lock, typename... dependencies>
    module_task module<TDerived, iservice_lock, dependencies...>::generate_task() {

        auto check_fn = [](const flecs::world& ecs) -> bool {
            return (true && ... && dependencies::is_ready(ecs));
        };

        auto init_fn = [this](flecs::world& ecs) -> void {

            if constexpr (!std::is_same_v<iservice_lock, no_lock>) {
                if (ecs.has<iservice_lock>()) {
                    throw std::runtime_error("Module conflict: Service lock already exists for " + this->name);
                }
            }

            ecs.module<TDerived>();
            this->on_initialize(ecs);
        };

        // Return the packaged lambdas to your bootstrapper
        return module_task {
            this->name,
            std::move(check_fn),
            std::move(init_fn)
        };
    }

    template<typename TDerived, typename iservice_lock, typename... dependencies>
    template <typename TDependencyLock>
    void module<TDerived, iservice_lock, dependencies...>::require_version(world& ecs, uint8_t min_major, uint8_t min_minor) const {
        if (!ecs.has<TDependencyLock>()) {
            throw std::runtime_error(name + " failed: Dependency missing!");
        }

        const auto& dep_service = ecs.get<TDependencyLock>();

        bool is_valid = (dep_service.version_major > min_major) ||
                        (dep_service.version_major == min_major && dep_service.version_minor >= min_minor);

        if (!is_valid) throw std::runtime_error(name + " failed version check!");
    }

} // namespace sandbox