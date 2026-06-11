#pragma once

#include <stdexcept>
#include <string>

namespace sandbox {

    class boot_error : public std::runtime_error {
    public:
        explicit boot_error(const std::string& message) : std::runtime_error(message) {}
        explicit boot_error(const char* message) : std::runtime_error(message) {}
    };

    class manifest_error : public std::runtime_error {
    public:
        explicit manifest_error(const std::string& message) : std::runtime_error(message) {}
        explicit manifest_error(const char* message) : std::runtime_error(message) {}
    };

    class vfs_error : public std::runtime_error {
    public:
        explicit vfs_error(const std::string& message) : std::runtime_error(message) {}
        explicit vfs_error(const char* message) : std::runtime_error(message) {}
    };

    class null_api_error : public std::runtime_error {
    public:
        explicit null_api_error(const std::string& message) : std::runtime_error(message) {}
        explicit null_api_error(const char* message) : std::runtime_error(message) {}
    };

} // namespace sandbox
