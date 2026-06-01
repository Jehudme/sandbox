#pragma once

#include <filesystem>
#include <string>

namespace sandbox::events::plugins {

    struct load_request {
        std::filesystem::path virtual_path;
        std::string entry_point = "SandboxLibraryMain";
    };

} // namespace sandbox::events::plugins