#pragma once

// Launcher-side PCH to cover what sandbox exposes publicly via its link interface.
// (sandbox links PUBLIC to flecs_static, RTTR::Core, spdlog::spdlog, glaze::glaze)

// Standard library (keep minimal, add more only if launcher uses them a lot)
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <filesystem>

#include <flecs.h>
#include <rttr/registration>
#include <rttr/type>

#include <spdlog/spdlog.h>
#include <glaze/glaze.hpp>