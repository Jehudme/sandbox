#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/properties.h"
#include <cstring>
#include <string>

TEST_CASE("Properties C API: Basic Lifecycle & Flat Setters/Getters", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    REQUIRE(props != nullptr);

    const char* key_int[] = {"my_int"};
    sandbox_properties_set_int64(props, key_int, 1, 42);

    const char* key_double[] = {"my_double"};
    sandbox_properties_set_double(props, key_double, 1, 3.14);

    const char* key_bool[] = {"my_bool"};
    sandbox_properties_set_bool(props, key_bool, 1, true);

    const char* key_str[] = {"my_string"};
    sandbox_properties_set_string(props, key_str, 1, "hello");

    int64_t out_int = 0;
    REQUIRE(sandbox_properties_get_int64(props, key_int, 1, &out_int));
    REQUIRE(out_int == 42);

    double out_double = 0.0;
    REQUIRE(sandbox_properties_get_double(props, key_double, 1, &out_double));
    REQUIRE(out_double == 3.14);

    bool out_bool = false;
    REQUIRE(sandbox_properties_get_bool(props, key_bool, 1, &out_bool));
    REQUIRE(out_bool == true);

    const char* out_str = sandbox_properties_get_string(props, key_str, 1);
    REQUIRE(out_str != nullptr);
    REQUIRE(std::strcmp(out_str, "hello") == 0);

    // Type-Safety Check
    REQUIRE_FALSE(sandbox_properties_get_int64(props, key_str, 1, &out_int));

    sandbox_properties_destroy(props);
}

TEST_CASE("Properties C API: Nested Paths", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    REQUIRE(props != nullptr);

    const char* path[] = {"engine", "window", "width"};
    sandbox_properties_set_int64(props, path, 3, 1920);

    int64_t width = 0;
    REQUIRE(sandbox_properties_get_int64(props, path, 3, &width));
    REQUIRE(width == 1920);

    sandbox_properties_destroy(props);
}

TEST_CASE("Properties C API: Tree Manipulation (_has, _clear, _keys)", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    
    const char* path_ab[] = {"a", "b"};
    sandbox_properties_set_int64(props, path_ab, 2, 1);
    
    const char* path_ac[] = {"a", "c"};
    sandbox_properties_set_int64(props, path_ac, 2, 2);

    const char* path_x[] = {"x"};
    sandbox_properties_set_int64(props, path_x, 1, 3);

    REQUIRE(sandbox_properties_has(props, path_ab, 2));
    
    sandbox_properties_clear(props, path_ab, 2);
    REQUIRE_FALSE(sandbox_properties_has(props, path_ab, 2));
    REQUIRE(sandbox_properties_has(props, path_ac, 2));

    const char* path_a[] = {"a"};
    size_t out_count = 0;
    char** keys = sandbox_properties_keys(props, path_a, 1, &out_count);
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

        const char* path_a_x[] = {"x"};
        sandbox_properties_set_int64(props_a, path_a_x, 1, 100);

        const char* path_b_y[] = {"y"};
        sandbox_properties_set_int64(props_b, path_b_y, 1, 200);

        const char* root_path[] = {nullptr};
        sandbox_properties_merge(props_a, root_path, 0, props_b); 

        int64_t out_y = 0;
        REQUIRE(sandbox_properties_get_int64(props_a, path_b_y, 1, &out_y));
        REQUIRE(out_y == 200);

        sandbox_properties_destroy(props_a);
        sandbox_properties_destroy(props_b);
    }

    SECTION("Sub") {
        sandbox_properties_t* props = sandbox_properties_create();
        const char* path_health[] = {"player", "stats", "health"};
        sandbox_properties_set_int64(props, path_health, 3, 100);

        const char* path_player[] = {"player"};
        sandbox_properties_t* sub_props = sandbox_properties_sub(props, path_player, 1);
        REQUIRE(sub_props != nullptr);

        const char* path_sub_health[] = {"stats", "health"};
        int64_t health = 0;
        REQUIRE(sandbox_properties_get_int64(sub_props, path_sub_health, 2, &health));
        REQUIRE(health == 100);

        sandbox_properties_destroy(props);
        sandbox_properties_destroy(sub_props);
    }
}

TEST_CASE("Properties C API: Serialization (_load and _dump)", "[properties][c_api]") {
    sandbox_properties_t* props = sandbox_properties_create();
    const char* path_nested[] = {"app", "config", "debug"};
    sandbox_properties_set_bool(props, path_nested, 3, true);

    SECTION("JSON Format") {
        char* dumped_json = sandbox_properties_dump(props, SANDBOX_FORMAT_JSON);
        REQUIRE(dumped_json != nullptr);
        REQUIRE(std::strlen(dumped_json) > 0);

        sandbox_properties_t* loaded_props = sandbox_properties_create();
        bool load_success = sandbox_properties_load(loaded_props, dumped_json, std::strlen(dumped_json), SANDBOX_FORMAT_JSON);
        REQUIRE(load_success);

        bool debug_val = false;
        REQUIRE(sandbox_properties_get_bool(loaded_props, path_nested, 3, &debug_val));
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
        REQUIRE(sandbox_properties_get_bool(loaded_props, path_nested, 3, &debug_val));
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

    const char* path[] = {"key"};
    REQUIRE_FALSE(sandbox_properties_has(nullptr, path, 1));

    int64_t dummy_int = 0;
    REQUIRE_FALSE(sandbox_properties_get_int64(nullptr, path, 1, &dummy_int));
    double dummy_double = 0.0;
    REQUIRE_FALSE(sandbox_properties_get_double(nullptr, path, 1, &dummy_double));
    bool dummy_bool = false;
    REQUIRE_FALSE(sandbox_properties_get_bool(nullptr, path, 1, &dummy_bool));
    REQUIRE(sandbox_properties_get_string(nullptr, path, 1) == nullptr);

    // Setters with null object
    REQUIRE_NOTHROW(sandbox_properties_set_int64(nullptr, path, 1, 42));
    REQUIRE_NOTHROW(sandbox_properties_set_double(nullptr, path, 1, 3.14));
    REQUIRE_NOTHROW(sandbox_properties_set_bool(nullptr, path, 1, true));
    REQUIRE_NOTHROW(sandbox_properties_set_string(nullptr, path, 1, "test"));

    // Setters with null path
    REQUIRE_NOTHROW(sandbox_properties_set_int64(props, nullptr, 1, 42));
    REQUIRE_NOTHROW(sandbox_properties_set_double(props, nullptr, 1, 3.14));
    REQUIRE_NOTHROW(sandbox_properties_set_bool(props, nullptr, 1, true));
    REQUIRE_NOTHROW(sandbox_properties_set_string(props, nullptr, 1, "test"));

    // String related with null path (gets root)
    const char* root_str = sandbox_properties_get_string(props, nullptr, 1);
    REQUIRE(root_str != nullptr);
    REQUIRE(sandbox_properties_get_string(props, path, 1) == nullptr); // Valid path, but key doesn't exist

    // Other functions with null parameters
    REQUIRE_NOTHROW(sandbox_properties_clear(nullptr, path, 1));
    REQUIRE_NOTHROW(sandbox_properties_clear(props, nullptr, 1));

    size_t count = 0;
    REQUIRE(sandbox_properties_keys(nullptr, path, 1, &count) == nullptr);
    REQUIRE(sandbox_properties_keys(props, nullptr, 1, &count) == nullptr);
    REQUIRE(sandbox_properties_keys(props, path, 1, nullptr) == nullptr);

    REQUIRE_NOTHROW(sandbox_properties_merge(nullptr, path, 1, props));
    REQUIRE_NOTHROW(sandbox_properties_merge(props, nullptr, 1, nullptr));
    REQUIRE_NOTHROW(sandbox_properties_merge(props, path, 1, nullptr));

    REQUIRE(sandbox_properties_sub(nullptr, path, 1) == nullptr);
    sandbox_properties_t* root_sub = sandbox_properties_sub(props, nullptr, 1);
    REQUIRE(root_sub != nullptr);
    sandbox_properties_destroy(root_sub);

    REQUIRE(sandbox_properties_dump(nullptr, SANDBOX_FORMAT_JSON) == nullptr);
    REQUIRE_FALSE(sandbox_properties_load(nullptr, "{}", 2, SANDBOX_FORMAT_JSON));
    REQUIRE_FALSE(sandbox_properties_load(props, nullptr, 2, SANDBOX_FORMAT_JSON));

    // Valid object cleanup
    sandbox_properties_destroy(props);
}
