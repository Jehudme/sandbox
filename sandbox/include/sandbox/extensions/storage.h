#pragma once

#include "sandbox/core/extension.h"

namespace sanbox::extensions
{
    class storage : public sandbox::extension
    {
    public:
        template<typename type, typename... arguments_types>
        void create(std::string_view name, arguments_types&&... constructor_arguments);

        template<typename... arguments_types>
        void create(std::string_view name, std::string_view type, arguments_types&&... constructor_arguments); // build from registry

        void destroy(std::string_view name);

        template<typename type>
        type* get(std::string_view name);
        bool exists(std::string_view name) const;

    private:
        void* _get(std::string_view name);
    };
}