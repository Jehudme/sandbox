#include "sandbox/data/registry.h"
#include "sandbox/diagnostics/scoped_logger.h"
#include <rttr/string_view.h>
#include <rttr/type.h>

namespace sandbox
{
    void* registry::_internal_create_instance(std::string_view type_identifier, std::vector<rttr::argument> args)
    {
        rttr::string_view type_view{ type_identifier.data(), type_identifier.size() };
        rttr::type reflection_type = rttr::type::get_by_name(type_view);

        if (!reflection_type.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: Identifier '{}' is not registered.", type_identifier);
            return nullptr;
        }

        std::vector<rttr::type> arg_types;
        arg_types.reserve(args.size());
        for (const auto& arg : args)
        {
            arg_types.push_back(arg.get_type());
        }

        rttr::constructor ctor = reflection_type.get_constructor(arg_types);

        if (!ctor.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: No matching constructor found for '{}'.", type_identifier);
            return nullptr;
        }

        rttr::variant instance_variant = ctor.invoke_variadic(args);

        if (!instance_variant.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: Failed to create instance of '{}'.", type_identifier);
            return nullptr;
        }

        return instance_variant.get_value<void*>();
    }

    rttr::variant registry::_internal_invoke(std::string_view identifier, void* instance, std::vector<rttr::argument> args)
    {
        rttr::string_view identifier_view{ identifier.data(), identifier.size() };
        rttr::method global_method = rttr::type::get_global_method(identifier_view);

        if (global_method.is_valid())
        {
            return global_method.invoke_variadic({}, args);
        }

        if (instance)
        {
            rttr::variant instance_var = instance;
            rttr::type t = instance_var.get_type();

            if (t.is_valid())
            {
                rttr::method member_method = t.get_method(identifier_view);
                if (member_method.is_valid())
                {
                    return member_method.invoke_variadic(instance_var, args);
                }
            }
        }

        SANDBOX_LOG_ERROR("Registry: Could not find function '{}'.", identifier);
        return rttr::variant();
    }

    rttr::variant registry::_internal_invoke_static(std::string_view class_identifier, std::string_view function_identifier, std::vector<rttr::argument> args)
    {
        rttr::string_view class_view{ class_identifier.data(), class_identifier.size() };
        rttr::type type = rttr::type::get_by_name(class_view);

        if (!type.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: Class '{}' not found.", class_identifier);
            return rttr::variant();
        }

        rttr::string_view function_view{ function_identifier.data(), function_identifier.size() };
        rttr::method method = type.get_method(function_view);

        if (method.is_valid())
        {
            return method.invoke_variadic({}, args);
        }

        SANDBOX_LOG_ERROR("Registry: Static method '{}' not found in '{}'.", function_identifier, class_identifier);
        return rttr::variant();
    }

    bool registry::is_type_registered(std::string_view type_identifier)
    {
        rttr::string_view type_view{ type_identifier.data(), type_identifier.size() };
        return rttr::type::get_by_name(type_view).is_valid();
    }

    std::string registry::get_registered_name(std::string_view type_identifier)
    {
        rttr::string_view type_view{ type_identifier.data(), type_identifier.size() };
        rttr::type reflection_type = rttr::type::get_by_name(type_view);

        if (reflection_type.is_valid())
        {
            return reflection_type.get_name().to_string();
        }

        SANDBOX_LOG_WARN("Registry: Identifier '{}' is unknown.", type_identifier);
        return "UNKNOWN_TYPE";
    }
}
