#include <catch2/catch_test_macros.hpp>
#include <sandbox/sdk/engine.hpp>
#include <sandbox/sdk/filesystem.hpp>
#include <filesystem>
#include <iostream>

TEST_CASE("Dir copy", "[filesystem]") {
    sandbox::engine engine;
    sandbox::properties props;
    props.set("booting-configuration/mount-path", "./test_mutations_dir");
    props.set_array("engine/sandbox", std::vector<std::string>{"sandbox-filesystem@1.0.0"});
    REQUIRE(engine.initialize(props));
    flecs::world world(static_cast<ecs_world_t*>(engine.get_ecs()));

    std::filesystem::create_directories("./test_mutations_dir/cache/src_dir");
    std::ofstream("./test_mutations_dir/cache/src_dir/file.txt") << "hello";

    bool res = sandbox::modules::filesystem::copy(world, "cache://src_dir", "cache://dest_dir", false, true);
    std::cout << "copy res: " << res << "\n";
    REQUIRE(res);
}
