// unit/src/properties/test_properties_getters_setters.cpp
//
// Unit tests for Properties typed get<T>() and set<T>():
//   - set/get int64_t (stored as double internally)
//   - set/get double
//   - set/get bool
//   - set/get std::string
//   - deep nested paths
//   - type mismatch returns nullopt
//   - overwrite an existing key with a different type
//   - get on an empty path (root node)

#include <catch2/catch_all.hpp>
#include "core/properties.h"

using sandbox::core::Properties;

// ---------------------------------------------------------------------------
// Feature: Properties — Integer (int64_t) get and set
// ---------------------------------------------------------------------------
TEST_CASE("Prop:set/get: int64_t round-trip",
          "[properties][getters_setters][int64]")
{
    Properties properties;
    properties.set<int64_t>({"count"}, 42);

    SECTION("value is retrievable as int64_t") {
        auto retrieved_value = properties.get<int64_t>({"count"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == 42);
    }

    SECTION("value is also retrievable as double (stored as double internally)") {
        auto retrieved_value = properties.get<double>({"count"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(42.0));
    }

    SECTION("get as bool returns nullopt (int and bool are distinct types)") {
        auto retrieved_value = properties.get<bool>({"count"});
        REQUIRE_FALSE(retrieved_value.has_value());
    }

    SECTION("large int64_t value survives round-trip") {
        properties.set<int64_t>({"large"}, 9'000'000);
        auto retrieved_value = properties.get<int64_t>({"large"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == 9'000'000);
    }

    SECTION("negative int64_t value survives round-trip") {
        properties.set<int64_t>({"negative"}, -1024);
        auto retrieved_value = properties.get<int64_t>({"negative"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == -1024);
    }

    SECTION("zero value survives round-trip") {
        properties.set<int64_t>({"zero"}, 0);
        auto retrieved_value = properties.get<int64_t>({"zero"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == 0);
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Double get and set
// ---------------------------------------------------------------------------
TEST_CASE("Prop:set/get: double round-trip",
          "[properties][getters_setters][double]")
{
    Properties properties;
    properties.set<double>({"pi"}, 3.14159);

    SECTION("value is retrievable with expected precision") {
        auto retrieved_value = properties.get<double>({"pi"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(3.14159));
    }

    SECTION("negative double round-trips correctly") {
        properties.set<double>({"delta"}, -0.001);
        auto retrieved_value = properties.get<double>({"delta"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(-0.001));
    }

    SECTION("zero double round-trips correctly") {
        properties.set<double>({"zero"}, 0.0);
        auto retrieved_value = properties.get<double>({"zero"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(0.0));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Boolean get and set
// ---------------------------------------------------------------------------
TEST_CASE("Prop:set/get: bool round-trip",
          "[properties][getters_setters][bool]")
{
    Properties properties;

    SECTION("true value survives round-trip") {
        properties.set<bool>({"flag_true"}, true);
        auto retrieved_value = properties.get<bool>({"flag_true"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == true);
    }

    SECTION("false value survives round-trip") {
        properties.set<bool>({"flag_false"}, false);
        auto retrieved_value = properties.get<bool>({"flag_false"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == false);
    }

    SECTION("bool is not retrievable as double") {
        properties.set<bool>({"flag"}, true);
        auto retrieved_as_double = properties.get<double>({"flag"});
        REQUIRE_FALSE(retrieved_as_double.has_value());
    }

    SECTION("bool is not retrievable as int64_t") {
        properties.set<bool>({"flag"}, true);
        auto retrieved_as_int = properties.get<int64_t>({"flag"});
        REQUIRE_FALSE(retrieved_as_int.has_value());
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — String get and set
// ---------------------------------------------------------------------------
TEST_CASE("Prop:set/get: std::string round-trip",
          "[properties][getters_setters][string]")
{
    Properties properties;

    SECTION("basic ASCII string survives round-trip") {
        std::string input_string = "hello world";
        properties.set<std::string>({"greeting"}, input_string);
        auto retrieved_value = properties.get<std::string>({"greeting"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == "hello world");
    }

    SECTION("empty string survives round-trip") {
        std::string empty_string;
        properties.set<std::string>({"empty"}, empty_string);
        auto retrieved_value = properties.get<std::string>({"empty"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(retrieved_value->empty());
    }

    SECTION("string with special characters survives round-trip") {
        std::string special_string = "path/to/some/resource";
        properties.set<std::string>({"path"}, special_string);
        auto retrieved_value = properties.get<std::string>({"path"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == "path/to/some/resource");
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Deep nested paths
// ---------------------------------------------------------------------------
TEST_CASE("Prop:set/get: deep nested path (3+ levels)",
          "[properties][getters_setters][nested]")
{
    Properties properties;
    properties.set<double>({"a", "b", "c"}, 99.0);

    SECTION("value is accessible at exact deep path") {
        auto retrieved_value = properties.get<double>({"a", "b", "c"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(99.0));
    }

    SECTION("intermediate nodes are objects (has returns true)") {
        REQUIRE(properties.has({"a"}));
        REQUIRE(properties.has({"a", "b"}));
        REQUIRE(properties.has({"a", "b", "c"}));
    }

    SECTION("sibling at same depth does not conflict") {
        properties.set<double>({"a", "b", "d"}, 100.0);
        REQUIRE(properties.get<double>({"a", "b", "c"}).value() == Catch::Approx(99.0));
        REQUIRE(properties.get<double>({"a", "b", "d"}).value() == Catch::Approx(100.0));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Overwrite existing key
// ---------------------------------------------------------------------------
TEST_CASE("Prop:set: overwriting an existing key replaces the v...",
          "[properties][getters_setters][overwrite]")
{
    Properties properties;
    properties.set<double>({"key"}, 1.0);

    SECTION("overwrite with same type") {
        properties.set<double>({"key"}, 2.0);
        auto retrieved_value = properties.get<double>({"key"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(2.0));
    }

    SECTION("overwrite numeric with bool") {
        properties.set<bool>({"key"}, true);
        // Old double is gone
        REQUIRE_FALSE(properties.get<double>({"key"}).has_value());
        // New bool is present
        auto retrieved_value = properties.get<bool>({"key"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == true);
    }

    SECTION("overwrite numeric with string") {
        std::string new_value = "replaced";
        properties.set<std::string>({"key"}, new_value);
        REQUIRE_FALSE(properties.get<double>({"key"}).has_value());
        REQUIRE(properties.get<std::string>({"key"}) == std::optional<std::string>("replaced"));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Missing key returns nullopt
// ---------------------------------------------------------------------------
TEST_CASE("Prop:get: missing key returns nullopt",
          "[properties][getters_setters][missing]")
{
    Properties properties;
    properties.set<double>({"existing"}, 5.0);

    REQUIRE_FALSE(properties.get<double>({"nonexistent"}).has_value());
    REQUIRE_FALSE(properties.get<int64_t>({"nonexistent"}).has_value());
    REQUIRE_FALSE(properties.get<bool>({"nonexistent"}).has_value());
    REQUIRE_FALSE(properties.get<std::string>({"nonexistent"}).has_value());
}
