#include "sandbox/utilities/filesystem.h"
#include "sandbox/core/platform.h"

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

    std::filesystem::path get_user_data_directory() {
        #if defined(SANDBOX_WINDOWS_PLATFORM)
            const char* appdata = std::getenv("APPDATA");
            if (!appdata) {
                throw std::runtime_error("[Filesystem] %APPDATA% environment variable is missing on Windows.");
            }
            return std::filesystem::path(appdata) / "Sandbox";

        #elif defined(SANDBOX_LINUX_PLATFORM)
            const char* xdg_data = std::getenv("XDG_DATA_HOME");
            if (xdg_data && *xdg_data != '\0') {
                return std::filesystem::path(xdg_data) / "sandbox";
            }

            const char* home = std::getenv("HOME");
            if (!home) {
                throw std::runtime_error("[Filesystem] $HOME environment variable is missing on Linux.");
            }
            return std::filesystem::path(home) / ".local" / "share" / "sandbox";

        #elif defined(SANDBOX_APPLE_PLATFORM)
            const char* home = std::getenv("HOME");
            if (!home) {
                throw std::runtime_error("[Filesystem] $HOME environment variable is missing on macOS.");
            }
            return std::filesystem::path(home) / "Library" / "Application Support" / "Sandbox";

        #endif
    }

    std::filesystem::path get_user_cache_directory() {
        #if defined(SANDBOX_WINDOWS_PLATFORM)
            const char* localappdata = std::getenv("LOCALAPPDATA");
            if (!localappdata) {
                throw std::runtime_error("[Filesystem] %LOCALAPPDATA% environment variable is missing on Windows.");
            }
            return std::filesystem::path(localappdata) / "Sandbox" / "Cache";

        #elif defined(SANDBOX_LINUX_PLATFORM)
            const char* xdg_cache = std::getenv("XDG_CACHE_HOME");
            if (xdg_cache && *xdg_cache != '\0') {
                return std::filesystem::path(xdg_cache) / "sandbox";
            }

            const char* home = std::getenv("HOME");
            if (!home) {
                throw std::runtime_error("[Filesystem] $HOME environment variable is missing on Linux.");
            }
            return std::filesystem::path(home) / ".cache" / "sandbox";

        #elif defined(SANDBOX_APPLE_PLATFORM)
            const char* home = std::getenv("HOME");
            if (!home) {
                throw std::runtime_error("[Filesystem] $HOME environment variable is missing on macOS.");
            }
            return std::filesystem::path(home) / "Library" / "Caches" / "Sandbox";

        #endif
    }
}