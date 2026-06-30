#pragma once

#include <stdexcept>
#include <string>

namespace sandbox::core {

    class sandbox_error : public std::runtime_error {
    public:
        explicit sandbox_error(const std::string& msg) : std::runtime_error(msg) {}
    };

    class library_load_error : public sandbox_error {
    public:
        explicit library_load_error(const std::string& msg) : sandbox_error(msg) {}
    };

    class module_activation_error : public sandbox_error {
    public:
        explicit module_activation_error(const std::string& msg) : sandbox_error(msg) {}
    };

    class service_collision_error : public sandbox_error {
    public:
        explicit service_collision_error(const std::string& msg) : sandbox_error(msg) {}
    };

    class module_dependency_error : public sandbox_error {
    public:
        explicit module_dependency_error(const std::string& msg) : sandbox_error(msg) {}
    };

    class property_format_error : public sandbox_error {
    public:
        explicit property_format_error(const std::string& msg) : sandbox_error(msg) {}
    };

}
