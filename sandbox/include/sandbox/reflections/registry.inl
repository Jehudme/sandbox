#pragma once

#include <rttr/type.h>

namespace sandbox
{
    template<typename base_type, typename... argument_types>
    std::unique_ptr<base_type> registry::create_type(std::string_view type_identifier, argument_types&&... constructor_arguments)
    {
        std::vector<rttr::argument> packed_arguments = { std::forward<argument_types>(constructor_arguments)... };
        void* raw_pointer = _internal_create_instance(type_identifier, std::move(packed_arguments));

        if (raw_pointer)
        {
            return std::unique_ptr<base_type>(static_cast<base_type*>(raw_pointer));
        }

        return nullptr;
    }

    template<typename... argument_types>
    rttr::variant registry::invoke(std::string_view identifier, void* instance, argument_types&&... args)
    {
        std::vector<rttr::argument> packed_arguments = { std::forward<argument_types>(args)... };
        return _internal_invoke(identifier, instance, std::move(packed_arguments));
    }

    template<typename... argument_types>
    rttr::variant registry::invoke_static(std::string_view class_identifier, std::string_view function_identifier, argument_types&&... args)
    {
        std::vector<rttr::argument> packed_arguments = { std::forward<argument_types>(args)... };
        return _internal_invoke_static(class_identifier, function_identifier, std::move(packed_arguments));
    }
}