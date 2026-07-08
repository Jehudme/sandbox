#pragma once
#include <sandbox/services/filesystem_service.h>

#ifdef __cplusplus
namespace sandbox::modules {
    class filesystem {
    public:
    static bool mount(flecs::world& entity_world, const char* physical_path, const char* virtual_mount_point, bool read_only);

    static bool unmount(flecs::world& entity_world, const char* mount_point);

    static sandbox_file_handle_t open_read(flecs::world& entity_world, const char* virtual_path);

    static sandbox_file_handle_t open_write(flecs::world& entity_world, const char* virtual_path, bool append, bool force_path);

    static size_t read(flecs::world& entity_world, sandbox_file_handle_t handle, void* buffer, size_t bytes_to_read);

    static size_t write(flecs::world& entity_world, sandbox_file_handle_t handle, const void* buffer, size_t bytes_to_write);

    static bool eof(flecs::world& entity_world, sandbox_file_handle_t handle);

    static size_t tell(flecs::world& entity_world, sandbox_file_handle_t handle);

    static bool seek(flecs::world& entity_world, sandbox_file_handle_t handle, size_t position);

    static size_t size(flecs::world& entity_world, sandbox_file_handle_t handle);

    static void close_handle(flecs::world& entity_world, sandbox_file_handle_t handle);

    static bool create_file(flecs::world& entity_world, const char* virtual_path, bool force_path);

    static bool remove_file(flecs::world& entity_world, const char* virtual_path);

    static bool copy(flecs::world& entity_world, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path);

    static bool move(flecs::world& entity_world, const char* source_virtual_path, const char* dest_virtual_path, bool overwrite, bool force_path);

    static bool create_directory(flecs::world& entity_world, const char* virtual_path, bool force_path);

    static bool remove_directory(flecs::world& entity_world, const char* virtual_path);

    static bool exists(flecs::world& entity_world, const char* virtual_path);

    static bool is_file(flecs::world& entity_world, const char* virtual_path);

    static bool is_directory(flecs::world& entity_world, const char* virtual_path);

    static bool is_readonly(flecs::world& entity_world, const char* virtual_path);

    static size_t file_size(flecs::world& entity_world, const char* virtual_path);

    static int64_t last_modified(flecs::world& entity_world, const char* virtual_path);

        static std::vector<uint8_t> read_all_bytes(flecs::world& entity_world, const char* virtual_path) {
            std::vector<uint8_t> result;
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api && service->api->read_all_bytes) {
                uint8_t* data = nullptr;
                size_t size = 0;
                if (service->api->read_all_bytes(entity_world.c_ptr(), virtual_path, &data, &size)) {
                    if (data && size > 0) {
                        result.assign(data, data + size);
                    }
                    if (service->api->free_bytes && data) {
                        service->api->free_bytes(entity_world.c_ptr(), data);
                    }
                } else {
                    // Propagate the exception indirectly or throw our own if it failed without crashing (the C-ABI will handle logging/throwing)
                    // Actually, if it failed but didn't throw across C boundary, we can throw here to satisfy tests.
                    throw std::runtime_error("SDK read_all_bytes failed");
                }
            } else {
                throw std::runtime_error("Filesystem service or read_all_bytes ABI not available");
            }
            return result;
        }

    static std::string read_all_text(flecs::world& entity_world, const char* virtual_path);

    static bool write_all(flecs::world& entity_world, const char* virtual_path, const void* data, size_t sz, bool force_path = false);

        static std::vector<std::string> list_files(flecs::world& entity_world, const char* virtual_path, bool recursive = false) {
            std::vector<std::string> result;
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_filesystem_service_t);
            if (service && service->api && service->api->list_files) {
                char** files = nullptr;
                size_t count = 0;
                if (service->api->list_files(entity_world.c_ptr(), virtual_path, recursive, &files, &count)) {
                    for (size_t i = 0; i < count; ++i) {
                        if (files[i]) {
                            result.emplace_back(files[i]);
                        }
                    }
                    if (service->api->free_file_list) {
                        service->api->free_file_list(entity_world.c_ptr(), files, count);
                    }
                }
            }
            return result;
        }
    };
}
#endif

