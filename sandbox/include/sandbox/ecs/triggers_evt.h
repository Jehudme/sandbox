#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <flecs.h>

namespace sandbox::extensions { class triggers; }

namespace sandbox::ecs
{
    /**
     * @brief Events and Commands for the triggers extension.
     */
    template<typename... components>
    struct create_trigger_evt
    {
        std::string name;
        std::function<void(sandbox::extensions::triggers&, std::string_view)> action;

        static create_trigger_evt create(std::string_view name,
                                         std::function<void(sandbox::extensions::triggers&, std::string_view)> action)
        {
            return {std::string(name), std::move(action)};
        }
    };

    struct destroy_trigger_evt
    {
        std::string name;

        static destroy_trigger_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct enable_trigger_evt
    {
        std::string name;

        static enable_trigger_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct disable_trigger_evt
    {
        std::string name;

        static disable_trigger_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct trigger_exists_evt
    {
        std::string name;
        bool* out_exists;

        static trigger_exists_evt create(std::string_view name, bool* out_exists)
        {
            return {std::string(name), out_exists};
        }
    };

    struct trigger_enabled_evt
    {
        std::string name;
        bool* out_enabled;

        static trigger_enabled_evt create(std::string_view name, bool* out_enabled)
        {
            return {std::string(name), out_enabled};
        }
    };
}
