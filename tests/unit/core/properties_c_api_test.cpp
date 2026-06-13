#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/properties.h"
#include <cstring>
#include <string>

TEST_CASE("Properties C API: Basic Lifecycle & Flat Setters/Getters", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    REQUIRE(props != nullptr);

    sandbox_properties_set_int64(props, "my_int", 42);
    sandbox_properties_set_double(props, "my_double", 3.14);
    sandbox_properties_set_bool(props, "my_bool", true);
    sandbox_properties_set_string(props, "my_string", "hello");

    int64_t out_int = 0;
    REQUIRE(sandbox_properties_get_int64(props, "my_int", &out_int));
    REQUIRE(out_int == 42);

    double out_double = 0.0;
    REQUIRE(sandbox_properties_get_double(props, "my_double", &out_double));
    REQUIRE(out_double == 3.14);

    bool out_bool = false;
    REQUIRE(sandbox_properties_get_bool(props, "my_bool", &out_bool));
    REQUIRE(out_bool == true);

    const char* out_str = sandbox_properties_get_string(props, "my_string");
    REQUIRE(out_str != nullptr);
    REQUIRE(std::strcmp(out_str, "hello") == 0);

    // Type-Safety Check
    REQUIRE_FALSE(sandbox_properties_get_int64(props, "my_string", &out_int));

    sandbox_properties_destroy(props);
}

TEST_CASE("Properties C API: Nested Paths", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    REQUIRE(props != nullptr);

    sandbox_properties_set_int64(props, "engine/window/width", 1920);

    int64_t width = 0;
    REQUIRE(sandbox_properties_get_int64(props, "engine/window/width", &width));
    REQUIRE(width == 1920);

    sandbox_properties_destroy(props);
}

TEST_CASE("Properties C API: Tree Manipulation (_has, _clear, _keys)", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    
    sandbox_properties_set_int64(props, "a/b", 1);
    sandbox_properties_set_int64(props, "a/c", 2);
    sandbox_properties_set_int64(props, "x", 3);

    REQUIRE(sandbox_properties_has(props, "a/b"));
    
    sandbox_properties_clear(props, "a/b");
    REQUIRE_FALSE(sandbox_properties_has(props, "a/b"));
    REQUIRE(sandbox_properties_has(props, "a/c"));

    size_t out_count = 0;
    char** keys = sandbox_properties_keys(props, "a", &out_count);
    REQUIRE(out_count == 1);
    REQUIRE(keys != nullptr);
    REQUIRE(std::strcmp(keys[0], "c") == 0);
    
    sandbox_properties_free_keys(keys, out_count);
    sandbox_properties_destroy(props);
}

TEST_CASE("Properties C API: Advanced Tree Operations (_merge and _sub)", "[properties][c_api]") {
    SECTION("Merge") {
        sandbox_properties_t* props_a = sandbox_properties_create();
        sandbox_properties_t* props_b = sandbox_properties_create();

        sandbox_properties_set_int64(props_a, "x", 100);
        sandbox_properties_set_int64(props_b, "y", 200);

        sandbox_properties_merge(props_a, nullptr, props_b); 

        int64_t out_y = 0;
        REQUIRE(sandbox_properties_get_int64(props_a, "y", &out_y));
        REQUIRE(out_y == 200);

        sandbox_properties_destroy(props_a);
        sandbox_properties_destroy(props_b);
    }

    SECTION("Sub") {
        sandbox_properties_t* props = sandbox_properties_create();
        sandbox_properties_set_int64(props, "player/stats/health", 100);

        sandbox_properties_t* sub_props = sandbox_properties_sub(props, "player");
        REQUIRE(sub_props != nullptr);

        int64_t health = 0;
        REQUIRE(sandbox_properties_get_int64(sub_props, "stats/health", &health));
        REQUIRE(health == 100);

        sandbox_properties_destroy(props);
        sandbox_properties_destroy(sub_props);
    }
}

TEST_CASE("Properties C API: Serialization (_load and _dump)", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    sandbox_properties_set_bool(props, "app/config/debug", true);

    SECTION("JSON Format") {
        char* dumped_json = sandbox_properties_dump(props, SANDBOX_FORMAT_JSON);
        REQUIRE(dumped_json != nullptr);
        REQUIRE(std::strlen(dumped_json) > 0);

        sandbox_properties_t* loaded_props = sandbox_properties_create();
        bool load_success = sandbox_properties_load(loaded_props, dumped_json, std::strlen(dumped_json), SANDBOX_FORMAT_JSON);
        REQUIRE(load_success);

        bool debug_val = false;
        REQUIRE(sandbox_properties_get_bool(loaded_props, "app/config/debug", &debug_val));
        REQUIRE(debug_val == true);

        sandbox_properties_free_string(dumped_json);
        sandbox_properties_destroy(loaded_props);
    }

    SECTION("TOML Format") {
        char* dumped_toml = sandbox_properties_dump(props, SANDBOX_FORMAT_TOML);
        REQUIRE(dumped_toml != nullptr);
        REQUIRE(std::strlen(dumped_toml) > 0);

        sandbox_properties_t* loaded_props = sandbox_properties_create();
        bool load_success = sandbox_properties_load(loaded_props, dumped_toml, std::strlen(dumped_toml), SANDBOX_FORMAT_TOML);
        REQUIRE(load_success);

        bool debug_val = false;
        REQUIRE(sandbox_properties_get_bool(loaded_props, "app/config/debug", &debug_val));
        REQUIRE(debug_val == true);

        sandbox_properties_free_string(dumped_toml);
        sandbox_properties_destroy(loaded_props);
    }

    sandbox_properties_destroy(props);
}

TEST_CASE("Properties C API: Edge Cases & Null Pointers", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    
    // Test null properties object
    REQUIRE_NOTHROW(sandbox_properties_destroy(nullptr));

    const char* path = "key";
    REQUIRE_FALSE(sandbox_properties_has(nullptr, path));

    int64_t dummy_int = 0;
    REQUIRE_FALSE(sandbox_properties_get_int64(nullptr, path, &dummy_int));
    double dummy_double = 0.0;
    REQUIRE_FALSE(sandbox_properties_get_double(nullptr, path, &dummy_double));
    bool dummy_bool = false;
    REQUIRE_FALSE(sandbox_properties_get_bool(nullptr, path, &dummy_bool));
    REQUIRE(sandbox_properties_get_string(nullptr, path) == nullptr);

    // Setters with null object
    REQUIRE_NOTHROW(sandbox_properties_set_int64(nullptr, path, 42));
    REQUIRE_NOTHROW(sandbox_properties_set_double(nullptr, path, 3.14));
    REQUIRE_NOTHROW(sandbox_properties_set_bool(nullptr, path, true));
    REQUIRE_NOTHROW(sandbox_properties_set_string(nullptr, path, "test"));

    // Setters with null path
    REQUIRE_NOTHROW(sandbox_properties_set_int64(props, nullptr, 42));
    REQUIRE_NOTHROW(sandbox_properties_set_double(props, nullptr, 3.14));
    REQUIRE_NOTHROW(sandbox_properties_set_bool(props, nullptr, true));
    REQUIRE_NOTHROW(sandbox_properties_set_string(props, nullptr, "test"));

    // String related with null path (gets root)
    const char* root_str = sandbox_properties_get_string(props, nullptr);
    REQUIRE(root_str != nullptr);
    REQUIRE(sandbox_properties_get_string(props, path) == nullptr); // Valid path, but key doesn't exist

    // Other functions with null parameters
    REQUIRE_NOTHROW(sandbox_properties_clear(nullptr, path));
    REQUIRE_NOTHROW(sandbox_properties_clear(props, nullptr));

    size_t count = 0;
    REQUIRE(sandbox_properties_keys(nullptr, path, &count) == nullptr);
    REQUIRE(sandbox_properties_keys(props, nullptr, &count) == nullptr);
    REQUIRE(sandbox_properties_keys(props, path, nullptr) == nullptr);

    REQUIRE_NOTHROW(sandbox_properties_merge(nullptr, path, props));
    REQUIRE_NOTHROW(sandbox_properties_merge(props, nullptr, nullptr));
    REQUIRE_NOTHROW(sandbox_properties_merge(props, path, nullptr));

    REQUIRE(sandbox_properties_sub(nullptr, path) == nullptr);
    sandbox_properties_t* root_sub = sandbox_properties_sub(props, nullptr);
    REQUIRE(root_sub != nullptr);
    sandbox_properties_destroy(root_sub);

    REQUIRE(sandbox_properties_dump(nullptr, SANDBOX_FORMAT_JSON) == nullptr);
    REQUIRE_FALSE(sandbox_properties_load(nullptr, "{}", 2, SANDBOX_FORMAT_JSON));
    REQUIRE_FALSE(sandbox_properties_load(props, nullptr, 2, SANDBOX_FORMAT_JSON));

    // Valid object cleanup
    sandbox_properties_destroy(props);
}
