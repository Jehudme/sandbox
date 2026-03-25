#pragma once

// Keep this file "stable": prefer headers that rarely change.
// Avoid including your own project headers here (they change often and will invalidate the PCH).

// -----------------------------
// C++ standard library
// -----------------------------
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <exception>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <filesystem>
#include <system_error>

// -----------------------------
// Common platform / build config
// -----------------------------
#if defined(__linux__)
  #include <unistd.h>
#endif

// -----------------------------
// Dependencies
// -----------------------------
#include <spdlog/spdlog.h>

// RTTR
#include <rttr/registration>
#include <rttr/type>

// Flecs (C API include; your project currently includes <flecs.h>)
#include <flecs.h>

// Glaze (pick one; include what you actually use)
// If this is too heavy / causes long PCH builds, remove it.
#include <glaze/glaze.hpp>