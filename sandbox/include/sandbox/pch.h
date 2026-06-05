#pragma once

// ── Standard Library ─────────────────────────────────────────────────────────
// Ordered: C compatibility, fundamental types, memory, string, containers, I/O,
// concurrency, filesystem, error handling, utilities.

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <new>

#include <string>
#include <string_view>

#include <array>
#include <vector>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include <functional>
#include <optional>
#include <expected>
#include <variant>
#include <tuple>

#include <algorithm>
#include <type_traits>
#include <utility>
#include <limits>

#include <chrono>
#include <atomic>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <filesystem>
#include <system_error>
#include <exception>
#include <stdexcept>

#include <format>

// ── Third-Party ───────────────────────────────────────────────────────────────
#include <spdlog/spdlog.h>
#include <flecs.h>
#include <glaze/glaze.hpp>