#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <rttr/type>

namespace sandbox
{
    class TypeRegistry
    {
    public:
        TypeRegistry() = delete;
        ~TypeRegistry() = delete;

        template<typename base_t, typename... arguments_t>
        static std::unique_ptr<base_t> instantiate(std::string_view type_name, arguments_t&&... constructor_arguments);

        template<typename... arguments_t>
        static rttr::variant call_method(std::string_view method_name, void* instance_pointer, arguments_t&&... method_arguments);

        template<typename... arguments_t>
        static rttr::variant call_static_method(std::string_view class_name, std::string_view method_name, arguments_t&&... method_arguments);

        static bool has_type(std::string_view type_name);
        static std::string get_type_metadata_name(std::string_view type_name);

    private:
        static void* internal_instantiate_pointer(std::string_view type_name, std::vector<rttr::argument> arguments);
        static rttr::variant internal_call_method(std::string_view method_name, void* instance_pointer, std::vector<rttr::argument> arguments);
        static rttr::variant internal_call_static_method(std::string_view class_name, std::string_view method_name, std::vector<rttr::argument> arguments);
    };
}

#include "TypeRegistry.inl"
