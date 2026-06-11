#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/plugin.h"
#include "sandbox/core/bootstrapper.h"
#include "sandbox/core/platform.h"
#include "utilities/loader.h"
#include <sandbox/modules/filesystem/filesystem_api.h>
#include <filesystem>
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

    struct mock_fs {
        static void execute_command_cb(void* instance, uint32_t command_id, const uint8_t* payload_data, size_t size) {
            using namespace sandbox::schemas::filesystem;
            if (static_cast<FilesystemCommand>(command_id) != FilesystemCommand_Absolute) return;
            
            auto args = flatbuffers::GetRoot<AbsoluteArgs>(payload_data);
            std::string path_str = args->virtual_path()->str();
            
            if (path_str.find("test_lib_mock") != std::string::npos) {
                std::vector<std::string> paths = {
                    "./test_lib_mock" + std::string(LIB_EXT),
                    "./bin/test_lib_mock" + std::string(LIB_EXT),
                    "../bin/test_lib_mock" + std::string(LIB_EXT),
                    (std::filesystem::current_path() / "cmake-build-debug" / "bin" / "test_lib_mock").string() + LIB_EXT,
                    (std::filesystem::current_path() / "cmake-build-debug-event-trace" / "bin" / "test_lib_mock").string() + LIB_EXT
                };
                for (const auto& p : paths) {
                    if (std::filesystem::exists(p)) {
                        path_str = p;
                        break;
                    }
                }
            }
            
            auto* out_payload = reinterpret_cast<sandbox_payload*>(args->out_payload_ptr());
            if (out_payload) {
                uint8_t* ptr = static_cast<uint8_t*>(std::malloc(path_str.size() + 1));
                std::memcpy(ptr, path_str.c_str(), path_str.size() + 1);
                out_payload->bytes = ptr;
                out_payload->size = path_str.size();
                out_payload->free_func = +[](void* p) { std::free(p); };
            }
            
            int32_t* res_ptr = reinterpret_cast<int32_t*>(args->out_result_ptr());
            if (res_ptr) *res_ptr = 0;
        }
    } mock;
    
    sandbox::filesystem_service fs_svc{};
    fs_svc.instance = &mock;
    fs_svc.execute_command = mock_fs::execute_command_cb;
    ecs.set<sandbox::filesystem_service>(fs_svc);

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
