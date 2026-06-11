#pragma once

namespace sandbox {
    struct vfs_paths {
        static constexpr const char* app_mount = "mount://app/";
        static constexpr const char* cache_mount = "mount://cache/";
        static constexpr const char* bin_mount = "mount://bin/";
        static constexpr const char* config_file = "mount://app/manifest.json";
        static constexpr const char* modules_cache_dir = "mount://cache/modules";
    };
}
