#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>
#include <flecs.h>
#include <flecs/addons/cpp/flecs.hpp>
#include <dylib.hpp>

namespace sandbox::core {

    /**
     * @brief Manages the loading and unloading of dynamic plugins (.so/.dll).
     */
    class library_loader_t {
    public:
        /**
         * @brief Constructs a new library loader instance.
         */
        library_loader_t() = default;

        /**
         * @brief Destroys the library loader instance.
         */
        ~library_loader_t() = default;

        library_loader_t(const library_loader_t&) = delete;
        library_loader_t& operator=(const library_loader_t&) = delete;
        library_loader_t(library_loader_t&&) noexcept = default;
        library_loader_t& operator=(library_loader_t&&) noexcept = default;

        /**
         * @brief Loads a dynamic library given a path and caches it.
         * @param entity_world The flecs world used for logging.
         * @param path The filesystem path to the library.
         */
        void load(flecs::world& entity_world, const std::filesystem::path& path);

        /**
         * @brief Explicitly unloads a library by name.
         * @param entity_world The flecs world used for logging.
         * @param library_name The name of the library to unload.
         */
        void unload(flecs::world& entity_world, const std::string& library_name);

    private:
        std::unordered_map<std::string, std::shared_ptr<dylib::library>> m_libraries;
    };

}