#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>
#include <flecs.h>
#include <flecs/addons/cpp/flecs.hpp>
#include <dylib.hpp>

namespace sandbox::core {

    class library_loader_t {
    public:
        library_loader_t() = default;
        ~library_loader_t() = default;

        library_loader_t(const library_loader_t&) = delete;
        library_loader_t& operator=(const library_loader_t&) = delete;
        library_loader_t(library_loader_t&&) noexcept = default;
        library_loader_t& operator=(library_loader_t&&) noexcept = default;

        // Loads a dynamic library given a path, caching it.
        // If already loaded, it does nothing but logs a warning.
        void load(flecs::world& ecs, const std::filesystem::path& path);

        // Explicitly unloads a library if it exists in the cache.
        void unload(flecs::world& ecs, const std::string& library_name);

    private:
        std::unordered_map<std::string, std::shared_ptr<dylib::library>> m_libraries;
    };

}