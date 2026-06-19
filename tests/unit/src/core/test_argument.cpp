#include <catch2/catch_all.hpp>
#include "core/engine.h"
#include "sandbox/core/argument.h"
#include <string>

using namespace sandbox::core;

TEST_CASE("Engine Arguments C API", "[engine][argument]") {
    properties_t props;
    props.set<int64_t>({"window", "width"}, 1920);
    props.set<double>({"physics", "gravity"}, 9.81);
    props.set<bool>({"debug", "enabled"}, true);
    props.set<std::string>({"graphics", "api"}, "Vulkan");

    engine_t engine;
    engine.initialize(props);
    
    ecs_world_t* ecs = engine.ecs.c_ptr();

    SECTION("sandbox_argument_has works") {
        REQUIRE(sandbox_argument_has(ecs, "window/width"));
        REQUIRE(!sandbox_argument_has(ecs, "window/height"));
    }

    SECTION("getters work correctly") {
        int64_t i = 0;
        REQUIRE(sandbox_argument_get_int64(ecs, "window/width", &i));
        REQUIRE(i == 1920);

        double d = 0.0;
        REQUIRE(sandbox_argument_get_double(ecs, "physics/gravity", &d));
        REQUIRE(d == 9.81);

        bool b = false;
        REQUIRE(sandbox_argument_get_bool(ecs, "debug/enabled", &b));
        REQUIRE(b == true);

        const char* s = sandbox_argument_get_string(ecs, "graphics/api");
        REQUIRE(s != nullptr);
        REQUIRE(std::string(s) == "Vulkan");
    }

    SECTION("keys work correctly") {
        size_t count = 0;
        char** keys = sandbox_argument_get_keys(ecs, "window", &count);
        REQUIRE(keys != nullptr);
        REQUIRE(count == 1);
        REQUIRE(std::string(keys[0]) == "width");
        sandbox_properties_free_keys(keys, count);
    }

    SECTION("subtree works correctly") {
        sandbox_properties_t* sub = sandbox_argument_get_subtree(ecs, "window");
        REQUIRE(sub != nullptr);
        
        int64_t w = 0;
        REQUIRE(sandbox_properties_get_int64(sub, "width", &w));
        REQUIRE(w == 1920);
        
        sandbox_properties_destroy(sub);
    }
}
