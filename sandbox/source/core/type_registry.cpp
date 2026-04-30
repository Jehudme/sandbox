#include "sandbox/core/type_registry.h"
#include <rttr/type>

namespace sandbox
{
    void* type_registry::internal_instantiate_pointer(std::string_view type_name, std::vector<rttr::argument> arguments)
    {
        const rttr::type target_type = rttr::type::get_by_name(std::string(type_name));
        if (!target_type.is_valid()) return nullptr;

        rttr::variant instance_variant = target_type.create(arguments);

        if (instance_variant.is_valid())
            if (instance_variant.get_type().is_pointer()) return instance_variant.get_value<void*>();

        return nullptr;
    }

    rttr::variant type_registry::internal_call_method(std::string_view method_name, rttr::instance instance, std::vector<rttr::argument> arguments)
    {
        if (!instance.is_valid()) return {};

        const rttr::type instance_type = instance.get_derived_type();

        const rttr::method target_method = instance_type.get_method(std::string(method_name));
        if (target_method.is_valid())
        {
            return target_method.invoke_variadic(instance, arguments);
        }

        return {};
    }

    rttr::variant type_registry::internal_call_static_method(std::string_view class_name, std::string_view method_name, std::vector<rttr::argument> arguments)
    {
        const rttr::type target_type = rttr::type::get_by_name(std::string(class_name));
        if (!target_type.is_valid()) return {};

        const rttr::method target_method = target_type.get_method(std::string(method_name));
        if (target_method.is_valid()) return target_method.invoke_variadic({}, arguments);

        return {};
    }

    bool type_registry::has_type(std::string_view type_name)
    {
        return rttr::type::get_by_name(std::string(type_name)).is_valid();
    }

    std::string type_registry::get_type_metadata_name(std::string_view type_name)
    {
        const rttr::type target_type = rttr::type::get_by_name(std::string(type_name));
        return target_type.is_valid() ? target_type.get_name().to_string() : std::string();
    }
}