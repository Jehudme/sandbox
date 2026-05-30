#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace sandbox::filesystem
{
    using namespace std::filesystem;

    bool is_file(const std::filesystem::path& path);
    bool is_directory(const std::filesystem::path& path);
    bool has_extension(const std::filesystem::path& path, const std::string& expected_ext);

    std::filesystem::path strip_extension(const std::filesystem::path& path);
    std::filesystem::path get_absolute_path(const std::filesystem::path& path);

    std::vector<std::filesystem::path> get_items_in_directory(const std::filesystem::path& dir_path);
    std::vector<std::filesystem::path> get_all_files_recursive(const std::filesystem::path& dir_path);
    std::vector<std::filesystem::path> get_all_directories_recursive(const std::filesystem::path& dir_path);
}