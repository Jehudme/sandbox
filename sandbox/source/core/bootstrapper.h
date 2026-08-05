#pragma once
#include <string_view>
#include <vector>
#include <filesystem>
#include <flecs.h>
#include "sandbox/abi/bootstrapper.h"
#include "library_loader.h"
struct bootstrapper_test_accessor;

namespace sandbox::core {
    using service_info_t = sandbox_service_info_t;
    using module_info_t = sandbox_module_info_t;

    /**
     * @brief Bootstrapper class responsible for staging, activating, and booting ABI plugins.
     */
    class bootstrapper_t {

    public:
        /**
         * @brief Constructs a new bootstrapper instance.
         */
        bootstrapper_t() = default;

        /**
         * @brief Destroys the bootstrapper instance.
         */
        ~bootstrapper_t() = default;

        /**
         * @brief Stages a service into the global registry.
         * @param service_info The ABI struct containing the service information.
         */
        static void stage_service(const service_info_t& service_info);

        /**
         * @brief Stages a module into the global registry.
         * @param module_info The ABI struct containing the module information.
         */
        static void stage_module(const module_info_t& module_info);

        /**
         * @brief Loads a dynamic library by loading it and executing its staged constructors.
         * @param entity_world The flecs world used for logging.
         * @param library_path The filesystem path to the dynamic library.
         */
        static void load_library(flecs::world& entity_world, const std::filesystem::path& library_path);

        /**
         * @brief Activates a module by its explicit architecture, name, and version.
         * @param entity_world The flecs world.
         * @param architecture The module architecture (e.g. 'sandbox').
         * @param name The module name.
         * @param version_major The major version.
         * @param version_minor The minor version.
         * @param version_patch The patch version (-1 for any).
         */
        void activate(flecs::world& entity_world, std::string_view architecture, std::string_view name, int version_major, int version_minor, int version_patch = -1);

        /**
         * @brief Activates a module by parsing its Unified Resource Name (URN).
         * @param entity_world The flecs world.
         * @param module_urn The module string in the format 'architecture-name@major.minor.patch'.
         */
        void activate(flecs::world& entity_world, std::string_view module_urn);

        /**
         * @brief Boots all activated plugins.
         * @param entity_world The flecs world.
         */
        void boot(flecs::world& entity_world);

        /**
         * @brief Gets the global library loader instance.
         * @return The library loader.
         */
        static library_loader_t& get_loader() { return m_loader; }

    private:
        friend struct ::bootstrapper_test_accessor;
        std::vector<module_info_t> m_active_modules;
        std::vector<module_info_t> m_booted_modules;

        static inline std::vector<service_info_t> m_services;
        static inline std::vector<module_info_t> m_modules;
        static inline library_loader_t m_loader;
    };
}
