#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <iostream>

#ifndef TEST_MOUNT_DIR
#error "TEST_MOUNT_DIR must be defined"
#endif

TEST_CASE("Launcher Integration Test", "[integration][launcher]") {
    std::filesystem::path app_zip = std::filesystem::path(TEST_MOUNT_DIR) / "test-app.zip";
    
    // We assume the launcher is built in the same directory structure as the tests
    // Usually it's in build/bin/sandbox
    // From tests/suite context, the current working directory during CTest might be the build dir or root
    // To be robust, let's find the launcher relative to this test executable or use a known path.
    // If the tests are run from build/bin, launcher is ./sandbox
    
    std::string launcher_path = "./sandbox";
    if (!std::filesystem::exists(launcher_path)) {
        launcher_path = "../bin/sandbox"; // If run from build/tests or somewhere
    }
    if (!std::filesystem::exists(launcher_path)) {
        launcher_path = "bin/sandbox"; 
    }

    SECTION("Launcher boots with test-app.zip successfully") {
        std::string command = launcher_path + " --mount " + app_zip.string() + " --dev";
        std::cout << "Executing: " << command << "\n";
        
        int result = std::system(command.c_str());
        
        // Return code should be 0 for success
        REQUIRE(result == 0);
    }
}
