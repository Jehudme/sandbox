#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <flecs.h>

namespace sandbox::extensions { class systems; }

namespace sandbox::ecs
{
    /**
     * @brief Events and Commands for the systems extension.
     *
     * The action callback is already bound to the concrete component types at creation
     * time, so the systems extension only needs to invoke it.
     */
    template<typename... components>
    struct create_system_evt
    {
        std::string name;
        std::string stage;
        std::function<void(sandbox::extensions::systems&, std::string_view, std::string_view)> action;

        static create_system_evt create(std::string_view name,
                                        std::string_view stage,
                                        std::function<void(sandbox::extensions::systems&, std::string_view, std::string_view)> action)
        {
            return {std::string(name), std::string(stage), std::move(action)};
        }
    };

    struct destroy_system_evt
    {
        std::string name;

        static destroy_system_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct enable_system_evt
    {
        std::string name;

        static enable_system_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct disable_system_evt
    {
        std::string name;

        static disable_system_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct system_exists_evt
    {
        std::string name;
        bool* out_exists;

        static system_exists_evt create(std::string_view name, bool* out_exists)
        {
            return {std::string(name), out_exists};
        }
    };

    struct system_enabled_evt
    {
        std::string name;
        bool* out_enabled;

        static system_enabled_evt create(std::string_view name, bool* out_enabled)
        {
            return {std::string(name), out_enabled};
        }
    };
}
