#pragma once

#include <expected>
#include <string_view>
#include "sandbox/core/ecs.h"

namespace sandbox::internal {

    /// Resolves a virtual path and dynamically links a plugin library into the ECS world.
    std::expected<void, std::string> load(world& ecs, std::string_view virtual_path);

} // namespace sandbox::internal
