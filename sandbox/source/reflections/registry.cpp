#include "../../include/sandbox/reflections/registry.h"
#include "../../include/sandbox/diagnostics/scoped_logger.h"
#include <rttr/type.h>

namespace sandbox
{
    void* registry::_internal_create_instance(const std::string& type_identifier)
    {
        rttr::type reflection_type = rttr::type::get_by_name(type_identifier);

        if (!reflection_type.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: Identifier '{}' does not exist in the reflection database.", type_identifier);
            return nullptr;
        }

        rttr::variant instance_variant = reflection_type.create();

        if (!instance_variant.is_valid())
        {
            SANDBOX_LOG_ERROR("Registry: Failed to create an instance of '{}'. Is a default constructor registered?", type_identifier);
            return nullptr;
        }

        if (instance_variant.is_type<void*>())
        {
            return instance_variant.get_value<void*>();
        }
        return instance_variant.get_value<void*>();
    }

    bool registry::is_type_registered(const std::string& type_identifier)
    {
        return rttr::type::get_by_name(type_identifier).is_valid();
    }

    std::string registry::get_registered_name(const std::string& type_identifier)
    {
        rttr::type reflection_type = rttr::type::get_by_name(type_identifier);
        if (reflection_type.is_valid())
        {
            return reflection_type.get_name().to_string();
        }
        SANDBOX_LOG_WARN("Registry: Identifier '{}' is not registered. Cannot retrieve name.", type_identifier);
        return "INVALID_NAME";
    }
}