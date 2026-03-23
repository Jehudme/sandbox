#pragma once

namespace sandbox
{
    template<typename base_type>
    std::unique_ptr<base_type> registry::create_type(const std::string& type_identifier)
    {
        void* raw_instance_pointer = _internal_create_instance(type_identifier);

        if (raw_instance_pointer)
        {
            return std::unique_ptr<base_type>(static_cast<base_type*>(raw_instance_pointer));
        }

        return nullptr;
    }
}