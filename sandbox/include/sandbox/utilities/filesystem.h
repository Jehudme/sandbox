#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include "sandbox/core/platform.h"

namespace sandbox::filesystem
{
    // NOTE: We intentionally do NOT `using namespace std::filesystem` here.
    // Doing so inside a public header bleeds std::filesystem into every consumer's
    // namespace via argument-dependent lookup, causing hard-to-diagnose ambiguities.

    SANDBOX_API bool is_file(const std::filesystem::path& path);
    SANDBOX_API bool is_directory(const std::filesystem::path& path);
    SANDBOX_API bool has_extension(const std::filesystem::path& path, const std::string& expected_ext);

    SANDBOX_API std::filesystem::path strip_extension(const std::filesystem::path& path);

    SANDBOX_API std::vector<std::filesystem::path> get_items_in_directory(const std::filesystem::path& dir_path);
    SANDBOX_API std::vector<std::filesystem::path> get_all_files_recursive(const std::filesystem::path& dir_path);
    SANDBOX_API std::vector<std::filesystem::path> get_all_directories_recursive(const std::filesystem::path& dir_path);

    SANDBOX_API std::filesystem::path get_user_data_directory();
    SANDBOX_API std::filesystem::path get_user_cache_directory();

    // current_path() delegates to std::filesystem::current_path(); exposed here
    // so callers don't need to bring in std::filesystem directly.
    inline std::filesystem::path current_path() {
        return std::filesystem::current_path();
    }
}