#pragma once
#include "sandbox/abi/bootstrapper.h"
#include <string>

namespace sandbox {
    class Bootstrapper {
    public:
        // Attaches to the bootstrapper instance for the given ecs world
        explicit Bootstrapper(ecs_world_t* ecs);
        explicit Bootstrapper(sandbox_bootstrapper_t* raw);

        void activate(const std::string& arch, const std::string& name, int major, int minor, int patch = -1);
        void activate(const std::string& module_str);
        void boot(ecs_world_t* ecs);

        sandbox_bootstrapper_t* get_raw() const { return m_bootstrapper; }

    private:
        sandbox_bootstrapper_t* m_bootstrapper;
    };
}

#include "detail/bootstrapper.inl"
