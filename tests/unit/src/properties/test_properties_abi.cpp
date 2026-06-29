// unit/src/properties/test_properties_abi.cpp
// Tests for the C ABI layer of Properties.

#include <catch2/catch_all.hpp>
#include "../../../../sandbox/include/sandbox/abi/properties.h"

#include <cstring>
#include <string>

TEST_CASE("PropABI: lifecycle (create/destroy)", "[properties][abi][lifecycle]")
{
    SECTION("create returns non-null") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        REQUIRE(!(!SANDBOX_HANDLE_IS_VALID(h)));
        sandbox_properties_destroy(&h);
    }

    SECTION("destroy with null is safe") {
        sandbox_properties_destroy(nullptr);
    }
}

TEST_CASE("PropABI: load and dump", "[properties][abi][load_dump]")
{
    SECTION("valid JSON load returns true") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        const std::string json = R"({"key":"value","count":42})";
        REQUIRE(sandbox_properties_load(h, json.c_str(), json.size(), SANDBOX_FORMAT_JSON) == true);
        sandbox_properties_destroy(&h);
    }

    SECTION("malformed JSON load returns false") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        const std::string bad = R"({malformed)";
        REQUIRE(sandbox_properties_load(h, bad.c_str(), bad.size(), SANDBOX_FORMAT_JSON) == false);
        sandbox_properties_destroy(&h);
    }

    SECTION("load with null handle returns false") {
        const std::string json = R"({"key":1})";
        REQUIRE(sandbox_properties_load(sandbox_properties_handle_t{0}, json.c_str(), json.size(), SANDBOX_FORMAT_JSON) == false);
    }

    SECTION("dump after load returns non-empty string") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        const std::string json = R"({"name":"sandbox"})";
        sandbox_properties_load(h, json.c_str(), json.size(), SANDBOX_FORMAT_JSON);
        char* out = sandbox_properties_dump(h, SANDBOX_FORMAT_JSON);
        REQUIRE(out != nullptr);
        REQUIRE(strlen(out) > 0);
        sandbox_properties_free_string(out);
        sandbox_properties_destroy(&h);
    }

    SECTION("dump with null handle returns null") {
        REQUIRE(sandbox_properties_dump(sandbox_properties_handle_t{0}, SANDBOX_FORMAT_JSON) == nullptr);
    }

    SECTION("free_string with null is safe") {
        REQUIRE_NOTHROW(sandbox_properties_free_string(nullptr));
    }
}

TEST_CASE("PropABI: has and clear", "[properties][abi][has_clear]")
{
    SECTION("has returns false for missing key") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        REQUIRE(sandbox_properties_has(h, "missing") == false);
        sandbox_properties_destroy(&h);
    }

    SECTION("has returns true after set") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_int64(h, "count", 5);
        REQUIRE(sandbox_properties_has(h, "count") == true);
        sandbox_properties_destroy(&h);
    }

    SECTION("clear removes a key") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_int64(h, "count", 5);
        sandbox_properties_clear(h, "count");
        REQUIRE(sandbox_properties_has(h, "count") == false);
        sandbox_properties_destroy(&h);
    }

    SECTION("clear with empty path clears root") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_int64(h, "a", 1);
        sandbox_properties_set_int64(h, "b", 2);
        sandbox_properties_clear(h, "");
        std::vector<std::string> keys;
        sandbox_properties_keys(h, "", [](const char* k, void* ctx) {
            static_cast<std::vector<std::string>*>(ctx)->emplace_back(k);
        }, &keys);
        
        REQUIRE(keys.size() == 0);
        sandbox_properties_destroy(&h);
    }
}

TEST_CASE("PropABI: keys enumeration", "[properties][abi][keys]")
{
    SECTION("empty object returns count 0") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        std::vector<std::string> keys;
        sandbox_properties_keys(h, "", [](const char* k, void* ctx) {
            static_cast<std::vector<std::string>*>(ctx)->emplace_back(k);
        }, &keys);
        REQUIRE(keys.size() == 0);
        sandbox_properties_destroy(&h);
    }

    SECTION("returns correct keys after setting values") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_int64(h, "alpha", 1);
        sandbox_properties_set_int64(h, "beta", 2);
        std::vector<std::string> keys;
        sandbox_properties_keys(h, "", [](const char* k, void* ctx) {
            static_cast<std::vector<std::string>*>(ctx)->emplace_back(k);
        }, &keys);
        REQUIRE(keys.size() == 2);
        std::sort(keys.begin(), keys.end());
        REQUIRE(keys[0] == "alpha");
        REQUIRE(keys[1] == "beta");
        sandbox_properties_destroy(&h);
    }
}

TEST_CASE("PropABI: merge and sub", "[properties][abi][merge_sub]")
{
    SECTION("merge copies source at given path") {
        sandbox_properties_handle_t dst = sandbox_properties_create();
        sandbox_properties_handle_t src = sandbox_properties_create();
        sandbox_properties_set_int64(src, "value", 42);
        sandbox_properties_merge(dst, "section", src);
        REQUIRE(sandbox_properties_has(dst, "section/value") == true);
        sandbox_properties_destroy(&src);
        sandbox_properties_destroy(&dst);
    }

    SECTION("sub extracts a sub-tree") {
        sandbox_properties_handle_t parent = sandbox_properties_create();
        sandbox_properties_set_int64(parent, "config/timeout", 30);
        sandbox_properties_handle_t sub{0};
        sub = sandbox_properties_sub(parent, "config");
        REQUIRE(!(!SANDBOX_HANDLE_IS_VALID(sub)));
        int64_t timeout = 0;
        REQUIRE(sandbox_properties_get_int64(sub, "timeout", &timeout) == true);
        REQUIRE(timeout == 30);
        sandbox_properties_destroy(&sub);
        sandbox_properties_destroy(&parent);
    }

    SECTION("sub with null handle returns null") {
        sandbox_properties_handle_t sub{0};
        sub = sandbox_properties_sub(sandbox_properties_handle_t{0}, "any");
        REQUIRE(!SANDBOX_HANDLE_IS_VALID(sub));
    }
}

TEST_CASE("PropABI: typed getters", "[properties][abi][getters]")
{
    SECTION("get_int64 returns correct value") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_int64(h, "score", 100);
        int64_t out = 0;
        REQUIRE(sandbox_properties_get_int64(h, "score", &out) == true);
        REQUIRE(out == 100);
        sandbox_properties_destroy(&h);
    }

    SECTION("get_int64 returns false for missing key") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        int64_t out = 0;
        REQUIRE(sandbox_properties_get_int64(h, "missing", &out) == false);
        sandbox_properties_destroy(&h);
    }

    SECTION("get_double returns correct value") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_double(h, "ratio", 0.75);
        double out = 0.0;
        REQUIRE(sandbox_properties_get_double(h, "ratio", &out) == true);
        REQUIRE(out == Catch::Approx(0.75));
        sandbox_properties_destroy(&h);
    }

    SECTION("get_bool returns correct value") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_bool(h, "active", true);
        bool out = false;
        REQUIRE(sandbox_properties_get_bool(h, "active", &out) == true);
        REQUIRE(out == true);
        sandbox_properties_destroy(&h);
    }

    SECTION("read_string returns correct value") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        sandbox_properties_set_string(h, "label", "engine");
        std::string s;
        sandbox_properties_read_string(h, "label", [](const char* val, void* ctx) {
            if (val) *static_cast<std::string*>(ctx) = val;
        }, &s);
        REQUIRE(s == "engine");
        sandbox_properties_destroy(&h);
    }

    SECTION("read_string returns null for missing key") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        bool was_null = false;
        sandbox_properties_read_string(h, "nonexistent", [](const char* val, void* ctx) {
            *static_cast<bool*>(ctx) = (val == nullptr);
        }, &was_null);
        REQUIRE(was_null == true);
        sandbox_properties_destroy(&h);
    }
}

TEST_CASE("PropABI: null safety", "[properties][abi][null_safety]")
{
    SECTION("setters with null handle are safe no-ops") {
        REQUIRE_NOTHROW(sandbox_properties_set_int64(sandbox_properties_handle_t{0}, "k", 1));
        REQUIRE_NOTHROW(sandbox_properties_set_double(sandbox_properties_handle_t{0}, "k", 1.0));
        REQUIRE_NOTHROW(sandbox_properties_set_bool(sandbox_properties_handle_t{0}, "k", true));
        REQUIRE_NOTHROW(sandbox_properties_set_string(sandbox_properties_handle_t{0}, "k", "v"));
    }

    SECTION("getters with null handle return false/null") {
        int64_t i = 0; double d = 0.0; bool b = false;
        REQUIRE(sandbox_properties_get_int64(sandbox_properties_handle_t{0}, "k", &i)    == false);
        REQUIRE(sandbox_properties_get_double(sandbox_properties_handle_t{0}, "k", &d)   == false);
        REQUIRE(sandbox_properties_get_bool(sandbox_properties_handle_t{0}, "k", &b)     == false);
        bool was_null = false;
        sandbox_properties_read_string(sandbox_properties_handle_t{0}, "k", [](const char* val, void* ctx) {
            *static_cast<bool*>(ctx) = (val == nullptr);
        }, &was_null);
        REQUIRE(was_null == true);
    }

    SECTION("getters with null out-pointer return false") {
        sandbox_properties_handle_t h = sandbox_properties_create();
        REQUIRE(sandbox_properties_get_int64(h, "k", nullptr)  == false);
        REQUIRE(sandbox_properties_get_double(h, "k", nullptr) == false);
        REQUIRE(sandbox_properties_get_bool(h, "k", nullptr)   == false);
        sandbox_properties_destroy(&h);
    }
}

TEST_CASE("PropABI: slash-path navigation", "[properties][abi][path]")
{
    sandbox_properties_handle_t h = sandbox_properties_create();
    sandbox_properties_set_string(h, "app/name", "sandbox_engine");
    sandbox_properties_set_double(h, "app/version", 1.0);

    SECTION("string reachable via slash path") {
        std::string s;
        sandbox_properties_read_string(h, "app/name", [](const char* val, void* ctx) {
            if (val) *static_cast<std::string*>(ctx) = val;
        }, &s);
        REQUIRE(s == "sandbox_engine");
    }

    SECTION("double reachable via slash path") {
        double v = 0.0;
        REQUIRE(sandbox_properties_get_double(h, "app/version", &v) == true);
        REQUIRE(v == Catch::Approx(1.0));
    }

    SECTION("has returns true for slash path") {
        REQUIRE(sandbox_properties_has(h, "app/name") == true);
        REQUIRE(sandbox_properties_has(h, "app/version") == true);
    }

    sandbox_properties_destroy(&h);
}
