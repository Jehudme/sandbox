#pragma once

#include "sandbox/core/ecs.h"

namespace sandbox::internal {

    std::expected<void, std::string> load(world& ecs, std::string_view virtual_path);

} // namespace sandbox::modules
