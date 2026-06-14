#include "bootstrapper.h"
#include "sandbox/core/bootstrapper.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <format>

namespace sandbox::core {

    void Bootstrapper::stage_service(const ServiceInfo& info)
    {
        auto it = std::find_if(
            m_services.begin(),
            m_services.end(),
            [&](const ServiceInfo& service) {
                return std::strcmp(service.name, info.name) == 0
                    && service.version_major == info.version_major
                    && service.version_minor == info.version_minor;
            });

        if (it != m_services.end()) {
            m_services.push_back(info);
        }
    }

    void Bootstrapper::stage_module(const ModuleInfo &info) {
        auto it = std::find_if(
            m_modules.begin(),
            m_modules.end(),
            [&](const ModuleInfo& module) {
                return std::strcmp(module.name, info.name) == 0
                    && module.version_major == info.version_major
                    && module.version_minor == info.version_minor;
            });

        if (it != m_modules.end()) {
           m_modules.push_back(info);
        }
    }

    void Bootstrapper::activate(std::string_view architecture, std::string_view name, int version_major, int version_minor, int version_patch) {
        auto it = m_active_modules.end();

        if (version_patch >= 0)
        {
            it = std::find_if(
                m_active_modules.begin(),
                m_active_modules.end(),
                [&](const ModuleInfo& module)
                {
                    return std::strcmp(module.name, name.data()) == 0
                        && module.version_major == version_major
                        && module.version_minor == version_minor
                        && module.version_patch == version_patch
                        && std::strcmp(module.architecture, architecture.data()) == 0;
                });
        }
        else
        {
            it = std::max_element(
                m_active_modules.begin(),
                m_active_modules.end(),
                [&](const ModuleInfo& a, const ModuleInfo& b)
                {
                    const bool a_match =
                        std::strcmp(a.name, name.data()) == 0 &&
                        a.version_major == version_major &&
                        a.version_minor == version_minor &&
                        std::strcmp(a.architecture, architecture.data()) == 0;

                    const bool b_match =
                        std::strcmp(b.name, name.data()) == 0 &&
                        b.version_major == version_major &&
                        b.version_minor == version_minor &&
                        std::strcmp(b.architecture, architecture.data()) == 0;

                    if (!a_match) return true;
                    if (!b_match) return false;

                    return a.version_patch < b.version_patch;
                });

            if (it != m_active_modules.end())
            {
                if (std::strcmp(it->name, name.data()) != 0 ||
                    it->version_major != version_major ||
                    it->version_minor != version_minor ||
                    std::strcmp(it->architecture, architecture.data()) != 0)
                {
                    it = m_active_modules.end();
                }
            }
        }

        if (it != m_active_modules.end()) {
            m_active_modules.push_back(*it);
        }
        else {
            std::string error_message = std::format("No matching module found to activate: {} v{}.{} (arch: {})", name, version_major, version_minor, architecture);
            throw std::invalid_argument(error_message);
        }
    }

    void Bootstrapper::boot(flecs::world &ecs) {
    }
}
