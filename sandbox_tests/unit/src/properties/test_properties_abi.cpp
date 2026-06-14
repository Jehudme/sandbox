// unit/src/properties/test_properties_abi.cpp
//
// Unit tests for the C ABI layer of Properties (sandbox/core/properties.h):
//   - sandbox_properties_create / sandbox_properties_destroy
//   - sandbox_properties_load / sandbox_properties_dump
//   - sandbox_properties_free_string
//   - sandbox_properties_has / sandbox_properties_clear
//   - sandbox_properties_keys / sandbox_properties_free_keys
//   - sandbox_properties_merge / sandbox_properties_sub
//   - sandbox_properties_get_* / sandbox_properties_set_*
//   - NULL-safety for all ABI functions

#include <catch2/catch_all.hpp>
#include "sandbox/core/properties.h"  // public C ABI header

#include <cstring>
#include <string>

// ---------------------------------------------------------------------------
// Feature: Properties ABI — Lifecycle (create / destroy)
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: create returns non-null",
          "[properties][abi][lifecycle]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    REQUIRE(properties_handle != nullptr);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: destroy with null pointer is safe (no crash)",
          "[properties][abi][lifecycle]")
{
    REQUIRE_NOTHROW(sandbox_properties_destroy(nullptr));
}

// ---------------------------------------------------------------------------
// Feature: Properties ABI — Load / Dump
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: load valid JSON returns true",
          "[properties][abi][load_dump]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();

    const std::string json_data = R"({"key":"value","count":42})";
    bool load_result = sandbox_properties_load(
        properties_handle,
        json_data.c_str(),
        json_data.size(),
        SANDBOX_FORMAT_JSON
    );

    REQUIRE(load_result == true);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: load malformed JSON returns false",
          "[properties][abi][load_dump]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();

    const std::string bad_json = R"({malformed)";
    bool load_result = sandbox_properties_load(
        properties_handle,
        bad_json.c_str(),
        bad_json.size(),
        SANDBOX_FORMAT_JSON
    );

    REQUIRE(load_result == false);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: load with null props returns false (null sa...",
          "[properties][abi][load_dump]")
{
    const std::string json_data = R"({"key":1})";
    bool load_result = sandbox_properties_load(nullptr, json_data.c_str(), json_data.size(), SANDBOX_FORMAT_JSON);
    REQUIRE(load_result == false);
}

TEST_CASE("PropABI: dump valid str after load",
          "[properties][abi][load_dump]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();

    const std::string json_data = R"({"name":"sandbox"})";
    sandbox_properties_load(properties_handle, json_data.c_str(), json_data.size(), SANDBOX_FORMAT_JSON);

    char* dumped_string = sandbox_properties_dump(properties_handle, SANDBOX_FORMAT_JSON);

    SECTION("dump returns non-null") {
        REQUIRE(dumped_string != nullptr);
    }

    SECTION("dump output is non-empty") {
        REQUIRE(strlen(dumped_string) > 0);
    }

    sandbox_properties_free_string(dumped_string);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: dump with null props returns null (null saf...",
          "[properties][abi][load_dump]")
{
    char* result = sandbox_properties_dump(nullptr, SANDBOX_FORMAT_JSON);
    REQUIRE(result == nullptr);
}

TEST_CASE("PropABI: free_string with null is safe",
          "[properties][abi][load_dump]")
{
    REQUIRE_NOTHROW(sandbox_properties_free_string(nullptr));
}

// ---------------------------------------------------------------------------
// Feature: Properties ABI — has / clear
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: has false if missing",
          "[properties][abi][has_clear]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    REQUIRE(sandbox_properties_has(properties_handle, "missing_key") == false);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: has returns true after setting a value",
          "[properties][abi][has_clear]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_int64(properties_handle, "count", 5);
    REQUIRE(sandbox_properties_has(properties_handle, "count") == true);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: clear removes a key",
          "[properties][abi][has_clear]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_int64(properties_handle, "count", 5);

    sandbox_properties_clear(properties_handle, "count");

    REQUIRE(sandbox_properties_has(properties_handle, "count") == false);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: clear with null path clears root",
          "[properties][abi][has_clear]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_int64(properties_handle, "a", 1);
    sandbox_properties_set_int64(properties_handle, "b", 2);

    // Empty string path maps to root clear in the C++ parse_path
    sandbox_properties_clear(properties_handle, "");

    size_t key_count = 0;
    char** returned_keys = sandbox_properties_keys(properties_handle, "", &key_count);

    SECTION("root is empty after clearing with empty path") {
        REQUIRE(key_count == 0);
        REQUIRE(returned_keys == nullptr);
    }

    sandbox_properties_free_keys(returned_keys, key_count);
    sandbox_properties_destroy(properties_handle);
}

// ---------------------------------------------------------------------------
// Feature: Properties ABI — keys / free_keys
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: keys null for empty",
          "[properties][abi][keys]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    size_t key_count = 999;  // sentinel — should be set to 0

    char** returned_keys = sandbox_properties_keys(properties_handle, "", &key_count);

    REQUIRE(key_count == 0);
    REQUIRE(returned_keys == nullptr);

    sandbox_properties_free_keys(returned_keys, key_count);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: keys correct keys",
          "[properties][abi][keys]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_int64(properties_handle, "alpha", 1);
    sandbox_properties_set_int64(properties_handle, "beta", 2);

    size_t key_count = 0;
    char** returned_keys = sandbox_properties_keys(properties_handle, "", &key_count);

    SECTION("count is 2") {
        REQUIRE(key_count == 2);
    }

    SECTION("returned pointer is non-null") {
        REQUIRE(returned_keys != nullptr);
    }

    if (returned_keys && key_count == 2) {
        std::vector<std::string> key_vector;
        for (size_t i = 0; i < key_count; ++i) {
            key_vector.emplace_back(returned_keys[i]);
        }
        std::sort(key_vector.begin(), key_vector.end());

        SECTION("both expected keys are present") {
            REQUIRE(key_vector[0] == "alpha");
            REQUIRE(key_vector[1] == "beta");
        }
    }

    sandbox_properties_free_keys(returned_keys, key_count);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: free_keys with null is safe",
          "[properties][abi][keys]")
{
    REQUIRE_NOTHROW(sandbox_properties_free_keys(nullptr, 5));
}

// ---------------------------------------------------------------------------
// Feature: Properties ABI — merge / sub
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: merge copies source into destination at giv...",
          "[properties][abi][merge_sub]")
{
    sandbox_properties_t* destination_handle = sandbox_properties_create();
    sandbox_properties_t* source_handle = sandbox_properties_create();

    sandbox_properties_set_int64(source_handle, "value", 42);
    sandbox_properties_merge(destination_handle, "section", source_handle);

    REQUIRE(sandbox_properties_has(destination_handle, "section/value") == true);

    sandbox_properties_destroy(source_handle);
    sandbox_properties_destroy(destination_handle);
}

TEST_CASE("PropABI: sub extracts a sub-tree into a new object",
          "[properties][abi][merge_sub]")
{
    sandbox_properties_t* parent_handle = sandbox_properties_create();
    sandbox_properties_set_int64(parent_handle, "config/timeout", 30);

    sandbox_properties_t* sub_handle = sandbox_properties_sub(parent_handle, "config");

    SECTION("sub handle is non-null") {
        REQUIRE(sub_handle != nullptr);
    }

    SECTION("value is accessible at relative path in sub") {
        int64_t retrieved_timeout = 0;
        bool get_result = sandbox_properties_get_int64(sub_handle, "timeout", &retrieved_timeout);
        REQUIRE(get_result == true);
        REQUIRE(retrieved_timeout == 30);
    }

    sandbox_properties_destroy(sub_handle);
    sandbox_properties_destroy(parent_handle);
}

TEST_CASE("PropABI: sub with null props returns null",
          "[properties][abi][merge_sub]")
{
    sandbox_properties_t* result = sandbox_properties_sub(nullptr, "any");
    REQUIRE(result == nullptr);
}

// ---------------------------------------------------------------------------
// Feature: Properties ABI — Getters
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: get_int64 returns correct",
          "[properties][abi][getters]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_int64(properties_handle, "score", 100);

    int64_t retrieved_value = 0;
    bool get_result = sandbox_properties_get_int64(properties_handle, "score", &retrieved_value);

    REQUIRE(get_result == true);
    REQUIRE(retrieved_value == 100);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: get_int64 false if missing",
          "[properties][abi][getters]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    int64_t out_value = 0;
    REQUIRE(sandbox_properties_get_int64(properties_handle, "missing", &out_value) == false);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: get_double returns correct",
          "[properties][abi][getters]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_double(properties_handle, "ratio", 0.75);

    double retrieved_value = 0.0;
    bool get_result = sandbox_properties_get_double(properties_handle, "ratio", &retrieved_value);

    REQUIRE(get_result == true);
    REQUIRE(retrieved_value == Catch::Approx(0.75));
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: get_bool returns correct",
          "[properties][abi][getters]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_bool(properties_handle, "active", true);

    bool retrieved_value = false;
    bool get_result = sandbox_properties_get_bool(properties_handle, "active", &retrieved_value);

    REQUIRE(get_result == true);
    REQUIRE(retrieved_value == true);
    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: get_string returns non-null string after set",
          "[properties][abi][getters]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    sandbox_properties_set_string(properties_handle, "label", "engine");

    const char* retrieved_string = sandbox_properties_get_string(properties_handle, "label");

    SECTION("returned pointer is non-null") {
        REQUIRE(retrieved_string != nullptr);
    }

    SECTION("returned string content matches") {
        REQUIRE(std::string(retrieved_string) == "engine");
    }

    sandbox_properties_destroy(properties_handle);
}

TEST_CASE("PropABI: get_string returns null for missing key",
          "[properties][abi][getters]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();
    const char* result = sandbox_properties_get_string(properties_handle, "nonexistent");
    REQUIRE(result == nullptr);
    sandbox_properties_destroy(properties_handle);
}

// ---------------------------------------------------------------------------
// Feature: Properties ABI — Null safety for getters/setters
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: setters with null props are safe no-ops",
          "[properties][abi][null_safety]")
{
    REQUIRE_NOTHROW(sandbox_properties_set_int64(nullptr, "key", 1));
    REQUIRE_NOTHROW(sandbox_properties_set_double(nullptr, "key", 1.0));
    REQUIRE_NOTHROW(sandbox_properties_set_bool(nullptr, "key", true));
    REQUIRE_NOTHROW(sandbox_properties_set_string(nullptr, "key", "val"));
}

TEST_CASE("PropABI: getters with null props or null output are ...",
          "[properties][abi][null_safety]")
{
    int64_t  out_int    = 0;
    double   out_double = 0.0;
    bool     out_bool   = false;

    REQUIRE(sandbox_properties_get_int64(nullptr, "key", &out_int)    == false);
    REQUIRE(sandbox_properties_get_double(nullptr, "key", &out_double) == false);
    REQUIRE(sandbox_properties_get_bool(nullptr, "key", &out_bool)    == false);
    REQUIRE(sandbox_properties_get_string(nullptr, "key")             == nullptr);

    // Null out pointer
    sandbox_properties_t* handle = sandbox_properties_create();
    REQUIRE(sandbox_properties_get_int64(handle, "key", nullptr)    == false);
    REQUIRE(sandbox_properties_get_double(handle, "key", nullptr)    == false);
    REQUIRE(sandbox_properties_get_bool(handle, "key", nullptr)      == false);
    sandbox_properties_destroy(handle);
}

// ---------------------------------------------------------------------------
// Feature: Properties ABI — Slash path navigation
// ---------------------------------------------------------------------------
TEST_CASE("PropABI: slash-delimited path navigates into nested ...",
          "[properties][abi][path]")
{
    sandbox_properties_t* properties_handle = sandbox_properties_create();

    sandbox_properties_set_string(properties_handle, "app/name", "sandbox_engine");
    sandbox_properties_set_double(properties_handle, "app/version", 1.0);

    SECTION("nested string is reachable via slash path") {
        const char* retrieved_name = sandbox_properties_get_string(properties_handle, "app/name");
        REQUIRE(retrieved_name != nullptr);
        REQUIRE(std::string(retrieved_name) == "sandbox_engine");
    }

    SECTION("nested double is reachable via slash path") {
        double retrieved_version = 0.0;
        bool get_result = sandbox_properties_get_double(properties_handle, "app/version", &retrieved_version);
        REQUIRE(get_result == true);
        REQUIRE(retrieved_version == Catch::Approx(1.0));
    }

    SECTION("has returns true for slash path") {
        REQUIRE(sandbox_properties_has(properties_handle, "app/name") == true);
        REQUIRE(sandbox_properties_has(properties_handle, "app/version") == true);
    }

    sandbox_properties_destroy(properties_handle);
}
