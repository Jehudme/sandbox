#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/plugin.h"
#include "sandbox/core/bootstrapper.h"
#include "sandbox/core/platform.h"
#include "utilities/loader.h"
#include "sandbox/subsystems/filesystem/ifilesystem.h"
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
        std::expected<void, std::string> mount(std::string_view physical_path, std::string_view virtual_prefix, bool read_only) override { return {}; }
        std::expected<void, std::string> unmount(std::string_view virtual_prefix) override { return {}; }
        std::expected<std::vector<std::byte>, std::string> read(std::string_view virtual_path) const override { return std::vector<std::byte>(); }
        std::expected<void, std::string> write(std::string_view virtual_path, std::vector<std::byte> data, bool append) override { return {}; }
        std::expected<std::vector<std::filesystem::path>, std::string> list(std::string_view virtual_path, bool recursive) const override { return std::vector<std::filesystem::path>(); }
        std::expected<void, std::string> remove(std::string_view virtual_path) override { return {}; }
        std::expected<void, std::string> mkdir(std::string_view virtual_path) override { return {}; }
        std::expected<void, std::string> rename(std::string_view old_virtual_path, std::string_view new_virtual_path) override { return {}; }
        std::expected<void, std::string> copy(std::string_view source_virtual_path, std::string_view destination_virtual_path) override { return {}; }
        std::expected<void, std::string> move(std::string_view source_virtual_path, std::string_view destination_virtual_path) override { return {}; }
        std::expected<sandbox::events::filesystem::file_metadata, std::string> state(std::string_view virtual_path) const override { return sandbox::events::filesystem::file_metadata{}; }
        std::expected<std::filesystem::path, std::string> absolute(std::string_view virtual_path) const override {
            return std::filesystem::current_path() / virtual_path;
        }
        void set_property(const std::string& key, const std::any& value) override {}
        std::any get_property(const std::string& key) const override { return {}; }
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
        REQUIRE(boot.activate("test_master"));
    }
}
