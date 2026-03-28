#include "../../include/sandbox/core/TypeRegistry.h"
#include <rttr/type>

namespace sandbox
{
    void* TypeRegistry::internal_instantiate_pointer(std::string_view type_name, std::vector<rttr::argument> arguments)
    {
        const rttr::type target_type = rttr::type::get_by_name(std::string(type_name));
        if (!target_type.is_valid()) return nullptr;

        rttr::variant instance_variant = target_type.create(arguments);

        if (instance_variant.is_valid())
            if (instance_variant.get_type().is_pointer()) return instance_variant.get_value<void*>();

        return nullptr;
    }

    rttr::variant TypeRegistry::internal_call_method(std::string_view method_name, void* instance_pointer, std::vector<rttr::argument> arguments)
    {
        if (!instance_pointer) return {};

        rttr::instance object_instance(instance_pointer);
        const rttr::type instance_type = object_instance.get_derived_type();

        const rttr::method target_method = instance_type.get_method(std::string(method_name));
        if (target_method.is_valid())
        {
            return target_method.invoke(object_instance, arguments);
        }

        return {};
    }

    rttr::variant TypeRegistry::internal_call_static_method(std::string_view class_name, std::string_view method_name, std::vector<rttr::argument> arguments)
    {
        const rttr::type target_type = rttr::type::get_by_name(std::string(class_name));
        if (!target_type.is_valid()) return {};

        const rttr::method target_method = target_type.get_method(std::string(method_name));
        if (target_method.is_valid()) return target_method.invoke({}, arguments);

        return {};
    }

    bool TypeRegistry::has_type(std::string_view type_name)
    {
        return rttr::type::get_by_name(std::string(type_name)).is_valid();
    }

    std::string TypeRegistry::get_type_metadata_name(std::string_view type_name)
    {
        const rttr::type target_type = rttr::type::get_by_name(std::string(type_name));
        return target_type.is_valid() ? target_type.get_name().to_string() : std::string();
    }
}