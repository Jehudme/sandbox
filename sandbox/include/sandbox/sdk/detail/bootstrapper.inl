#pragma once
#include "../bootstrapper.hpp"

namespace sandbox {

    inline bootstrapper::bootstrapper(ecs_world_t* ecs) {
        m_bootstrapper = sandbox_get_bootstrapper(ecs);
    }

    inline bootstrapper::bootstrapper(sandbox_bootstrapper_t* raw) : m_bootstrapper(raw) {}

    inline void bootstrapper::activate(const std::string& arch, const std::string& name, int major, int minor, int patch) {
        sandbox_bootstrapper_activate(m_bootstrapper, arch.c_str(), name.c_str(), major, minor, patch);
    }

    inline void bootstrapper::activate(const std::string& module_str) {
        sandbox_bootstrapper_activate_string(m_bootstrapper, module_str.c_str());
    }

    inline void bootstrapper::boot(ecs_world_t* ecs) {
        sandbox_bootstrapper_boot(m_bootstrapper, ecs);
    }

}
