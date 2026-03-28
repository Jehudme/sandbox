#include <rttr/type>
#include <utility>

namespace sandbox
{
    template<typename base_type, typename... argument_types>
    std::unique_ptr<base_type> TypeRegistry::instantiate(std::string_view type_name, argument_types&&... constructor_arguments)
    {
        std::vector<rttr::argument> argument_list({ std::forward<argument_types>(constructor_arguments)... });

        void* raw_instance_ptr = internal_instantiate_pointer(type_name, std::move(argument_list));
        if (raw_instance_ptr) return std::unique_ptr<base_type>(static_cast<base_type*>(raw_instance_ptr));

        return nullptr;
    }

    template<typename... argument_types>
    rttr::variant TypeRegistry::call_method(std::string_view method_name, void* instance_pointer, argument_types&&... method_arguments)
    {
        std::vector<rttr::argument> argument_list({ std::forward<argument_types>(method_arguments)... });
        return internal_call_method(method_name, instance_pointer, std::move(argument_list));
    }

    template<typename... argument_types>
    rttr::variant TypeRegistry::call_static_method(std::string_view class_name, std::string_view method_name, argument_types&&... method_arguments)
    {
        std::vector<rttr::argument> argument_list({ std::forward<argument_types>(method_arguments)... });
        return internal_call_static_method(class_name, method_name, std::move(argument_list));
    }
}