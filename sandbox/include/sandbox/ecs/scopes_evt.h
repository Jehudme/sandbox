#pragma once

#include <string>
#include <vector>

namespace sandbox::ecs
{
    struct set_scope_evt
    {
        std::vector<std::string> path;

        static set_scope_evt create(std::vector<std::string> path)
        {
            return {std::move(path)};
        }
    };

    struct push_scope_evt
    {
        std::vector<std::string> path;

        static push_scope_evt create(std::vector<std::string> path)
        {
            return {std::move(path)};
        }
    };
}
