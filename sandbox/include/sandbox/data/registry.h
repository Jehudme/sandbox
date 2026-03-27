#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace rttr
{
    class argument;
    class variant;
}

namespace sandbox
{
    class registry
    {
    public:
        registry() = delete;
        ~registry() = delete;

        template<typename base_type, typename... argument_types>
        static std::unique_ptr<base_type> create_type(std::string_view type_identifier, argument_types&&... constructor_arguments);

        template<typename... argument_types>
        static rttr::variant invoke(std::string_view identifier, void* instance, argument_types&&... args);

        template<typename... argument_types>
        static rttr::variant invoke_static(std::string_view class_identifier, std::string_view function_identifier, argument_types&&... args);

        static bool is_type_registered(std::string_view type_identifier);

        static std::string get_registered_name(std::string_view type_identifier);

    private:
        static void* _internal_create_instance(std::string_view type_identifier, std::vector<rttr::argument> args);
        static rttr::variant _internal_invoke(std::string_view identifier, void* instance, std::vector<rttr::argument> args);
        static rttr::variant _internal_invoke_static(std::string_view class_identifier, std::string_view function_identifier, std::vector<rttr::argument> args);
    };
}

#include "registry.inl"