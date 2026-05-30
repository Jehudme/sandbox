#include "sandbox/utilities/filesystem.h"

namespace sandbox::filesystem
{
    bool is_file(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    }

    bool is_directory(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    }

    bool has_extension(const std::filesystem::path& path, const std::string& expected_ext) {
        return is_file(path) && (path.extension().string() == expected_ext);
    }

    std::filesystem::path strip_extension(const std::filesystem::path& path) {
        return path.parent_path() / path.stem();
    }


    std::vector<std::filesystem::path> get_items_in_directory(const std::filesystem::path& dir_path) {
        std::vector<std::filesystem::path> items;
        if (!filesystem::is_directory(dir_path)) return items;

        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
            items.push_back(entry.path());
        }
        return items;
    }

    std::vector<std::filesystem::path> get_all_files_recursive(const std::filesystem::path& dir_path) {
        std::vector<std::filesystem::path> files;
        if (!filesystem::is_directory(dir_path)) return files;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                files.push_back(entry.path());
            }
        }
        return files;
    }

    std::vector<std::filesystem::path> get_all_directories_recursive(const std::filesystem::path& dir_path) {
        std::vector<std::filesystem::path> directories;
        if (!filesystem::is_directory(dir_path)) return directories;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path)) {
            if (std::filesystem::is_directory(entry.path())) {
                directories.push_back(entry.path());
            }
        }
        return directories;
    }
}