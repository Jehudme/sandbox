#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/plugin.h"
#include "sandbox/core/bootstrapper.h"
#include "sandbox/core/platform.h"
#include "utilities/loader.h"
#include "subsystems/filesystem/ifilesystem.h"
#include <string>

#ifdef _WIN32
    #define LIB_EXT ".dll"
#elif __APPLE__
    #define LIB_EXT ".dylib"
#else
    #define LIB_EXT ".so"
#endif

TEST_CASE("Plugin Loading (Integration)", "[integration][plugin]") {
    flecs::world ecs;
    ecs.import<sandbox::bootstrapper>();
    auto& boot = ecs.get_mut<sandbox::bootstrapper>();

    struct mock_fs : public sandbox::ifilesystem {
        int32_t mount(const char* physical_path, const char* virtual_prefix, bool read_only) override { return 0; }
        int32_t unmount(const char* virtual_prefix) override { return 0; }
        int32_t read(const char* virtual_path, sandbox_payload* out_payload) const override { return -1; }
        int32_t write(const char* virtual_path, const uint8_t* data, size_t size, bool append) override { return 0; }
        int32_t list(const char* virtual_path, bool recursive, sandbox_payload* out_payload) const override { return 0; }
        int32_t remove(const char* virtual_path) override { return 0; }
        int32_t mkdir(const char* virtual_path) override { return 0; }
        int32_t rename(const char* old_virtual_path, const char* new_virtual_path) override { return 0; }
        int32_t copy(const char* source_virtual_path, const char* destination_virtual_path) override { return 0; }
        int32_t move(const char* source_virtual_path, const char* destination_virtual_path) override { return 0; }
        int32_t state(const char* virtual_path, sandbox_payload* out_payload) const override { return 0; }
        int32_t absolute(const char* virtual_path, sandbox_payload* out_payload) const override {
            if (!out_payload) return -1;
            
            std::string path_str(virtual_path);
            if (path_str.find("test_lib_mock") != std::string::npos) {
                path_str = std::string("./test_lib_mock") + LIB_EXT;
            }
            
            uint8_t* ptr = static_cast<uint8_t*>(std::malloc(path_str.size() + 1));
            std::memcpy(ptr, path_str.c_str(), path_str.size() + 1);
            out_payload->bytes = ptr;
            out_payload->size = path_str.size();
            out_payload->free_func = [](void* p) { std::free(p); };
            return 0;
        }
        void set_property(const char* key, const char* json_value) override {}
        int32_t get_property(const char* key, sandbox_payload* out_payload) const override { return -1; }
    } mock;
    ecs.set<sandbox::filesystem_service>({&mock});

    std::string lib_path = std::string("modules/test_lib_mock") + LIB_EXT;

    SECTION("The engine locates the libtest shared library, mounts it, and correctly invokes SandboxLibraryMain") {
        // sandbox::internal::load uses ecs_import_from_library which invokes SandboxLibraryMain
        auto res = sandbox::internal::load(ecs, lib_path);
        REQUIRE(res.has_value());
    }

    SECTION("The engine populates the internal Bootstrapper array with the module_info from libtest") {
        auto res = sandbox::internal::load(ecs, lib_path);
        REQUIRE(res.has_value());
        
        // Since load worked, SandboxLibraryMain invoked boot.stage() successfully.
        // We can verify this by activating a known module from libtest.
        REQUIRE_NOTHROW(boot.activate("showcase_plugin"));
    }
}
