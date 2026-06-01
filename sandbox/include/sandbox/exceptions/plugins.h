#pragma once

#include <stdexcept>
#include <string>
#include <filesystem>
#include "sandbox/core/platform.h"

namespace sandbox::events::plugins {

    class SANDBOX_API plugin_error : public std::runtime_error {
    public:
        plugin_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
            : std::runtime_error(format_message(context, path, details)) {}

    private:
        static std::string format_message(const std::string& context, const std::filesystem::path& path, const std::string& details) {
            return "[" + context + "] " + path.generic_string() + ": " + details;
        }
    };

    class plugin_load_error : public plugin_error {
    public:
        plugin_load_error(const std::string& context, const std::filesystem::path& path, const std::string& details = "OS rejected linking.")
            : plugin_error("Plugin Load Error | " + context, path, details) {}
    };

} // namespace sandbox::events::plugins