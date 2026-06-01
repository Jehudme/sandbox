#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

#include "sandbox/core/ecs.h"
#include "sandbox/utilities/properties.h"

namespace sandbox {

    class engine {
    public:
        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const std::filesystem::path& root_mount_path);
        void finalize();

        void load_library(const std::filesystem::path& virtual_library_path);
        void load_module_from_library(const std::filesystem::path& physical_library_path, std::string_view module_name);

    public:
        flecs::world ecs;
    };

}