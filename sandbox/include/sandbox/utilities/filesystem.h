#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include "sandbox/core/platform.h"

namespace sandbox::filesystem
{
    using namespace std::filesystem;

    SANDBOX_API bool is_file(const std::filesystem::path& path);
    SANDBOX_API bool is_directory(const std::filesystem::path& path);
    SANDBOX_API bool has_extension(const std::filesystem::path& path, const std::string& expected_ext);

    SANDBOX_API std::filesystem::path strip_extension(const std::filesystem::path& path);
    SANDBOX_API std::filesystem::path get_absolute_path(const std::filesystem::path& path);

    SANDBOX_API std::vector<std::filesystem::path> get_items_in_directory(const std::filesystem::path& dir_path);
    SANDBOX_API std::vector<std::filesystem::path> get_all_files_recursive(const std::filesystem::path& dir_path);
    SANDBOX_API std::vector<std::filesystem::path> get_all_directories_recursive(const std::filesystem::path& dir_path);

    SANDBOX_API std::filesystem::path get_user_data_directory();
    SANDBOX_API std::filesystem::path get_user_cache_directory();
}