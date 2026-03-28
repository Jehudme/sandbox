#pragma once

#include <string>
#include <string_view>

namespace sandbox::io
{
    /**
     * @brief Events and Commands for the filesystem extension.
     */

    struct resolve_path_evt
    {
        std::string path;
        std::string* out_result;

        static resolve_path_evt create(std::string_view path, std::string* out_result)
        {
            return {std::string(path), out_result};
        }
    };
}

