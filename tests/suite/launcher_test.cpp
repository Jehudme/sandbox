#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <iostream>

#ifndef TEST_MOUNT_DIR
#error "TEST_MOUNT_DIR must be defined"
#endif

#ifndef LAUNCHER_EXEC_PATH
#error "LAUNCHER_EXEC_PATH must be defined"
#endif

TEST_CASE("Launcher Integration Test", "[integration][launcher]") {
    std::filesystem::path app_zip = std::filesystem::path(TEST_MOUNT_DIR) / "test-app.zip";
    std::string launcher_path = LAUNCHER_EXEC_PATH;

    SECTION("Launcher boots with test-app.zip successfully") {
        std::string command = launcher_path + " --mount " + app_zip.string() + " --dev";
        std::cout << "Executing: " << command << "\n";
        
        int result = std::system(command.c_str());
        
        // Return code should be 0 for success
        REQUIRE(result == 0);
    }
}
