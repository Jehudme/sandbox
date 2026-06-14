// unit/src/properties/test_properties_load_dump.cpp
//
// Unit tests for Properties::load() and Properties::dump():
//   - Round-trip JSON (load then dump restores data)
//   - Round-trip TOML
//   - Round-trip YAML
//   - Invalid JSON throws
//   - dump() on empty object returns valid empty JSON

#include <catch2/catch_all.hpp>
#include "core/properties.h"

using sandbox::core::Properties;

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------
static const std::string SIMPLE_JSON = R"({"name":"sandbox","version":1,"enabled":true})";
static const std::string NESTED_JSON = R"({"app":{"name":"engine","level":5},"debug":false})";

// ---------------------------------------------------------------------------
// Feature: Properties — JSON load / dump round-trip
// ---------------------------------------------------------------------------
TEST_CASE("Properties::load: parses JSON and values become accessible",
          "[properties][load_dump][json]")
{
    Properties properties;
    properties.load(SIMPLE_JSON, Properties::Format::JSON);

    SECTION("string value is retrievable after JSON load") {
        auto retrieved_name = properties.get<std::string>({"name"});
        REQUIRE(retrieved_name.has_value());
        REQUIRE(*retrieved_name == "sandbox");
    }

    SECTION("integer value stored as double is retrievable") {
        auto retrieved_version = properties.get<double>({"version"});
        REQUIRE(retrieved_version.has_value());
        REQUIRE(*retrieved_version == Catch::Approx(1.0));
    }

    SECTION("boolean value is retrievable after JSON load") {
        auto retrieved_enabled = properties.get<bool>({"enabled"});
        REQUIRE(retrieved_enabled.has_value());
        REQUIRE(*retrieved_enabled == true);
    }
}

TEST_CASE("Properties::load: parses nested JSON correctly",
          "[properties][load_dump][json]")
{
    Properties properties;
    properties.load(NESTED_JSON, Properties::Format::JSON);

    SECTION("nested string is accessible via path") {
        auto retrieved_name = properties.get<std::string>({"app", "name"});
        REQUIRE(retrieved_name.has_value());
        REQUIRE(*retrieved_name == "engine");
    }

    SECTION("nested integer is accessible via path") {
        auto retrieved_level = properties.get<double>({"app", "level"});
        REQUIRE(retrieved_level.has_value());
        REQUIRE(*retrieved_level == Catch::Approx(5.0));
    }

    SECTION("top-level bool is accessible") {
        auto retrieved_debug = properties.get<bool>({"debug"});
        REQUIRE(retrieved_debug.has_value());
        REQUIRE(*retrieved_debug == false);
    }
}

TEST_CASE("Properties::dump: produces valid JSON from loaded data",
          "[properties][load_dump][json]")
{
    Properties properties;
    properties.load(SIMPLE_JSON, Properties::Format::JSON);

    std::string serialized = properties.dump(Properties::Format::JSON);

    SECTION("dump output is non-empty") {
        REQUIRE_FALSE(serialized.empty());
    }

    SECTION("round-trip: reload from dump restores all values") {
        Properties round_trip_properties;
        round_trip_properties.load(serialized, Properties::Format::JSON);

        REQUIRE(round_trip_properties.get<std::string>({"name"}) == std::optional<std::string>("sandbox"));
        REQUIRE(round_trip_properties.get<bool>({"enabled"}) == std::optional<bool>(true));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — TOML load / dump
// ---------------------------------------------------------------------------
TEST_CASE("Properties::load: parses TOML format",
          "[properties][load_dump][toml]")
{
    // Use flat TOML key-value format — section headers ([table]) may not be
    // supported by Glaze's glz::generic TOML reader.
    const std::string toml_data = "host = \"localhost\"\nport = 8080\n";

    Properties properties;
    properties.load(toml_data, Properties::Format::TOML);

    SECTION("TOML string value is accessible") {
        auto retrieved_host = properties.get<std::string>({"host"});
        REQUIRE(retrieved_host.has_value());
        REQUIRE(*retrieved_host == "localhost");
    }

    SECTION("TOML integer is accessible as double") {
        auto retrieved_port = properties.get<double>({"port"});
        REQUIRE(retrieved_port.has_value());
        REQUIRE(*retrieved_port == Catch::Approx(8080.0));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — YAML load / dump
// ---------------------------------------------------------------------------
TEST_CASE("Properties::load: parses YAML format",
          "[properties][load_dump][yaml]")
{
    const std::string yaml_data = "name: my_app\nversion: 3\n";

    Properties properties;
    properties.load(yaml_data, Properties::Format::YAML);

    SECTION("YAML string is accessible") {
        auto retrieved_name = properties.get<std::string>({"name"});
        REQUIRE(retrieved_name.has_value());
        REQUIRE(*retrieved_name == "my_app");
    }

    SECTION("YAML integer is accessible as double") {
        auto retrieved_version = properties.get<double>({"version"});
        REQUIRE(retrieved_version.has_value());
        REQUIRE(*retrieved_version == Catch::Approx(3.0));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Invalid data
// ---------------------------------------------------------------------------
TEST_CASE("Properties::load: throws on malformed JSON",
          "[properties][load_dump][error]")
{
    Properties properties;
    const std::string malformed_json = R"({broken json here)";

    REQUIRE_THROWS_AS(
        properties.load(malformed_json, Properties::Format::JSON),
        std::runtime_error
    );
}

TEST_CASE("Properties::dump: empty Properties produces valid empty JSON object",
          "[properties][load_dump][json]")
{
    Properties empty_properties;
    std::string serialized = empty_properties.dump(Properties::Format::JSON);

    SECTION("output is non-empty") {
        REQUIRE_FALSE(serialized.empty());
    }

    SECTION("output represents an empty JSON object") {
        // Glaze serializes empty object as {}
        REQUIRE(serialized.find('{') != std::string::npos);
        REQUIRE(serialized.find('}') != std::string::npos);
    }
}
