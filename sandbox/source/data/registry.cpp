#include "sandbox/data/registry.h"
#include "sandbox/diagnostics/scoped_logger.h"
#include <rttr/type.h>

namespace sandbox
{
    void* registry::_internal_create_instance(std::string_view type_identifier, std::vector<rttr::argument> args)
    {
        rttr::type reflection_type = rttr::type::get_by_name(type_identifier.data());

        if (!reflection_type.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: Identifier '{}' is not registered.", type_identifier);
            return nullptr;
        }

        // FIX 1: reflection_type.create(args) treats the std::vector as the FIRST argument, causing the _Destroy template crash.
        // We must extract the types and invoke the constructor variadically.
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
        // FIX 2: Correct namespace for global methods is rttr::type
        rttr::method global_method = rttr::type::get_global_method(identifier.data());

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
                rttr::method member_method = t.get_method(identifier.data());
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
        rttr::type type = rttr::type::get_by_name(class_identifier.data());

        if (!type.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: Class '{}' not found.", class_identifier);
            return rttr::variant();
        }

        rttr::method method = type.get_method(function_identifier.data());

        if (method.is_valid())
        {
            return method.invoke_variadic({}, args);
        }

        SANDBOX_LOG_ERROR("Registry: Static method '{}' not found in '{}'.", function_identifier, class_identifier);
        return rttr::variant();
    }

    bool registry::is_type_registered(std::string_view type_identifier)
    {
        return rttr::type::get_by_name(type_identifier.data()).is_valid();
    }

    std::string registry::get_registered_name(std::string_view type_identifier)
    {
        rttr::type reflection_type = rttr::type::get_by_name(type_identifier.data());

        if (reflection_type.is_valid())
        {
            return reflection_type.get_name().to_string();
        }

        SANDBOX_LOG_WARN("Registry: Identifier '{}' is unknown.", type_identifier);
        return "UNKNOWN_TYPE";
    }
}