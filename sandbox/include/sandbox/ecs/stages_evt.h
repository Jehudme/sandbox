#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace sandbox::ecs
{
    struct create_stage_evt
    {
        std::string name;
        std::vector<std::string> dependencies;

        static create_stage_evt create(std::string_view name, std::vector<std::string> dependencies = {})
        {
            return {std::string(name), std::move(dependencies)};
        }
    };

    struct destroy_stage_evt
    {
        std::string name;

        static destroy_stage_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct enable_stage_evt
    {
        std::string name;

        static enable_stage_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct disable_stage_evt
    {
        std::string name;

        static disable_stage_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct stage_exists_evt
    {
        std::string name;
        bool* out_exists;

        static stage_exists_evt create(std::string_view name, bool* out_exists)
        {
            return {std::string(name), out_exists};
        }
    };

    struct stage_enabled_evt
    {
        std::string name;
        bool* out_enabled;

        static stage_enabled_evt create(std::string_view name, bool* out_enabled)
        {
            return {std::string(name), out_enabled};
        }
    };
}
