// unit/src/properties/test_properties_load_dump.cpp
// Tests for properties_t::load() and dump() across JSON, TOML, YAML.

#include <catch2/catch_all.hpp>
#include "core/properties.h"

using sandbox::core::properties_t;

static const std::string SIMPLE_JSON = R"({"name":"sandbox","version":1,"enabled":true})";
static const std::string NESTED_JSON = R"({"app":{"name":"engine","level":5},"debug":false})";

TEST_CASE("Prop: JSON load round-trip", "[properties][load_dump][json]")
{
    properties_t p;
    p.load(SIMPLE_JSON, properties_t::Format::JSON);

    SECTION("string value accessible") {
        REQUIRE(*p.get<std::string>({"name"}) == "sandbox");
    }

    SECTION("number accessible as double") {
        REQUIRE(*p.get<double>({"version"}) == Catch::Approx(1.0));
    }

    SECTION("bool accessible") {
        REQUIRE(*p.get<bool>({"enabled"}) == true);
    }

    SECTION("nested keys accessible") {
        properties_t q;
        q.load(NESTED_JSON, properties_t::Format::JSON);
        REQUIRE(*q.get<std::string>({"app", "name"}) == "engine");
        REQUIRE(*q.get<double>({"app", "level"}) == Catch::Approx(5.0));
        REQUIRE(*q.get<bool>({"debug"}) == false);
    }

    SECTION("dump then reload restores values") {
        std::string json = p.dump(properties_t::Format::JSON);
        REQUIRE_FALSE(json.empty());
        properties_t rt;
        rt.load(json, properties_t::Format::JSON);
        REQUIRE(rt.get<std::string>({"name"}) == std::optional<std::string>("sandbox"));
        REQUIRE(rt.get<bool>({"enabled"}) == std::optional<bool>(true));
    }

    SECTION("dump of empty object is valid") {
        properties_t empty;
        std::string out = empty.dump(properties_t::Format::JSON);
        REQUIRE_FALSE(out.empty());
        REQUIRE(out.find('{') != std::string::npos);
        REQUIRE(out.find('}') != std::string::npos);
    }
}

TEST_CASE("Prop: TOML and YAML load", "[properties][load_dump][formats]")
{
    SECTION("TOML flat key-values are accessible") {
        const std::string toml = "host = \"localhost\"\nport = 8080\n";
        properties_t p;
        p.load(toml, properties_t::Format::TOML);
        REQUIRE(*p.get<std::string>({"host"}) == "localhost");
        REQUIRE(*p.get<double>({"port"}) == Catch::Approx(8080.0));
    }

    SECTION("YAML key-values are accessible") {
        const std::string yaml = "name: my_app\nversion: 3\n";
        properties_t p;
        p.load(yaml, properties_t::Format::YAML);
        REQUIRE(*p.get<std::string>({"name"}) == "my_app");
        REQUIRE(*p.get<double>({"version"}) == Catch::Approx(3.0));
    }

    SECTION("malformed JSON throws runtime_error") {
        properties_t p;
        REQUIRE_THROWS_AS(
            p.load(R"({broken json here)", properties_t::Format::JSON),
            std::runtime_error
        );
    }
}
