#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include "sandbox/core/platform.h"

namespace sandbox {

    struct SANDBOX_API engine_arguments {
        // Core Engine Boot Requirements
         std::filesystem::path mounts;
        bool developer_mode{false};

        // Dynamic Payload: Passes custom Key=Value pairs down to plugins
        std::unordered_map<std::string, std::string> module_args;
    };

} // namespace sandbox