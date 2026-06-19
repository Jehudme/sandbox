#pragma once
#include <string_view>
#include <vector>
#include <filesystem>
#include <flecs.h>
#include "sandbox/core/bootstrapper.h"
#include "library_loader.h"

namespace sandbox::core {
    using service_info_t = sandbox_service_info_t;
    using module_info_t = sandbox_module_info_t;

    class bootstrapper_t {

    public:
        bootstrapper_t() = default;
        ~bootstrapper_t() = default;

        static void stage_service(const service_info_t& info);
        static void stage_module(const module_info_t& info);
        static void reset();

        static void index_library(const std::filesystem::path& library_path);

        void activate(std::string_view architecture, std::string_view name, int version_major, int version_minor, int version_patch = -1);
        void activate(std::string_view module_str);
        void boot(flecs::world& ecs);

    private:
        std::vector<module_info_t> m_active_modules;

        static inline std::vector<service_info_t> m_services;
        static inline std::vector<module_info_t> m_modules;
        static inline library_loader_t m_loader;
    };
}
