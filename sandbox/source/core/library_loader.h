#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>
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

        void load(const std::filesystem::path& path);
        void unload(const std::string& library_name);

    private:
        std::unordered_map<std::string, dylib::library> m_libraries;
    };

}