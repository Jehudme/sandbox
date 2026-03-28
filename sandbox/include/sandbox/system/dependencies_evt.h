#pragma once

#include <string>
#include <string_view>

namespace sandbox::system
{
    /**
     * @brief Events and Commands for the dependencies extension.
     */

    struct require_dependency_evt
    {
        std::string extension_name;

        static require_dependency_evt create(std::string_view extension_name)
        {
            return {std::string(extension_name)};
        }
    };

    struct validate_dependencies_evt
    {
        static validate_dependencies_evt create()
        {
            return {};
        }
    };
}

