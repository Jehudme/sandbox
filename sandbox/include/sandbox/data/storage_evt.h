#pragma once

#include <string>
#include <string_view>
#include <functional>

namespace sandbox::extensions { class storage; }

namespace sandbox::data
{
    template<typename base_type = void>
    struct create_object_evt
    {
        std::string name;
        std::function<void(sandbox::extensions::storage&, std::string_view)> action;

        static create_object_evt create(std::string_view name, std::function<void(sandbox::extensions::storage&, std::string_view)> action)
        {
            return {std::string(name), std::move(action)};
        }
    };

    struct destroy_object_evt
    {
        std::string name;

        static destroy_object_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    template<typename base_type = void>
    struct get_object_evt
    {
        std::string name;
        std::function<void(sandbox::extensions::storage&, std::string_view, base_type**)> action;
        base_type** out_object;

        static get_object_evt create(std::string_view name, base_type** out_object, std::function<void(sandbox::extensions::storage&, std::string_view, base_type**)> action)
        {
            return {std::string(name), std::move(action), out_object};
        }
    };

    struct object_exists_evt
    {
        std::string name;
        bool* out_exists;

        static object_exists_evt create(std::string_view name, bool* out_exists)
        {
            return {std::string(name), out_exists};
        }
    };
}
