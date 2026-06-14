#pragma once
#include <string_view>
#include <vector>
#include "sandbox/core/bootstrapper.h"

namespace sandbox::core {
    using ServiceInfo = sandbox_service_info_t;
    using ModuleInfo = sandbox_module_info_t;

    class Bootstrapper {

    public:
        Bootstrapper() = default;
        ~Bootstrapper() = default;

        static void stage_service(const ServiceInfo& info);
        static void stage_module(const ModuleInfo& info);
        static void reset();

        void activate(std::string_view architecture, std::string_view name, int version_major, int version_minor, int version_patch = -1);
        void boot(flecs::world& ecs);

    private:
        std::vector<ModuleInfo> m_active_modules;

        static inline std::vector<ServiceInfo> m_services;
        static inline std::vector<ModuleInfo> m_modules;
    };
}
