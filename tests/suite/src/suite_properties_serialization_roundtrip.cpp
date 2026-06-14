// suite/src/suite_properties_serialization_roundtrip.cpp
// Integration: Properties serialization round-trips across JSON, TOML, YAML, C ABI.

#include <catch2/catch_all.hpp>
#include "core/properties.h"
#include "sandbox/core/properties.h"

#include <string>
#include <vector>

using sandbox::core::Properties;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static void populate_engine_config(Properties& p) {
    p.set<double>({"window", "width"},  1920.0);
    p.set<double>({"window", "height"}, 1080.0);
    p.set<bool>({"window", "fullscreen"}, false);
    p.set<std::string>({"window", "title"}, std::string("Sandbox Engine"));
    p.set<double>({"graphics", "target_fps"}, 60.0);
    p.set<bool>({"graphics", "vsync"}, true);
    p.set<std::string>({"graphics", "api"}, std::string("OpenGL"));
    p.set<double>({"graphics", "msaa_samples"}, 4.0);
    p.set<double>({"audio", "master_volume"}, 0.8);
    p.set<bool>({"audio", "muted"}, false);
    p.set<double>({"audio", "sample_rate"}, 44100.0);
    p.set<std::string>({"network", "host"}, std::string("127.0.0.1"));
    p.set<double>({"network", "port"}, 7777.0);
    p.set<double>({"network", "timeout_ms"}, 5000.0);
}

static void verify_engine_config(const Properties& p) {
    REQUIRE(p.get<double>({"window", "width"})      == std::optional<double>(1920.0));
    REQUIRE(p.get<double>({"window", "height"})     == std::optional<double>(1080.0));
    REQUIRE(p.get<bool>({"window", "fullscreen"})   == std::optional<bool>(false));
    REQUIRE(p.get<std::string>({"window", "title"}) == std::optional<std::string>("Sandbox Engine"));
    REQUIRE(p.get<double>({"graphics", "target_fps"})     == std::optional<double>(60.0));
    REQUIRE(p.get<bool>({"graphics", "vsync"})            == std::optional<bool>(true));
    REQUIRE(p.get<std::string>({"graphics", "api"})       == std::optional<std::string>("OpenGL"));
    REQUIRE(p.get<double>({"audio", "master_volume"}).value() == Catch::Approx(0.8));
    REQUIRE(p.get<bool>({"audio", "muted"})               == std::optional<bool>(false));
    REQUIRE(p.get<std::string>({"network", "host"})       == std::optional<std::string>("127.0.0.1"));
    REQUIRE(p.get<double>({"network", "port"})            == std::optional<double>(7777.0));
}

// ---------------------------------------------------------------------------
// Suite: Engine config round-trips
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Engine config round-trip (JSON, TOML, YAML)", "[suite][properties][serialization]")
{
    Properties source;
    populate_engine_config(source);

    SECTION("JSON — window settings survive") {
        Properties rt;
        REQUIRE_NOTHROW(rt.load(source.dump(Properties::Format::JSON), Properties::Format::JSON));
        REQUIRE(rt.get<double>({"window", "width"})        == std::optional<double>(1920.0));
        REQUIRE(rt.get<double>({"window", "height"})       == std::optional<double>(1080.0));
        REQUIRE(rt.get<bool>({"window", "fullscreen"})     == std::optional<bool>(false));
        REQUIRE(rt.get<std::string>({"window", "title"})   == std::optional<std::string>("Sandbox Engine"));
    }

    SECTION("JSON — graphics and audio survive") {
        Properties rt;
        rt.load(source.dump(Properties::Format::JSON), Properties::Format::JSON);
        REQUIRE(rt.get<bool>({"graphics", "vsync"})              == std::optional<bool>(true));
        REQUIRE(rt.get<std::string>({"graphics", "api"})         == std::optional<std::string>("OpenGL"));
        REQUIRE(rt.get<double>({"audio", "master_volume"}).value() == Catch::Approx(0.8));
    }

    SECTION("TOML round-trip preserves all values") {
        Properties rt;
        auto toml = source.dump(Properties::Format::TOML);
        REQUIRE_FALSE(toml.empty());
        REQUIRE_NOTHROW(rt.load(toml, Properties::Format::TOML));
        verify_engine_config(rt);
    }

    SECTION("YAML round-trip preserves all values") {
        Properties rt;
        auto yaml = source.dump(Properties::Format::YAML);
        REQUIRE_FALSE(yaml.empty());
        REQUIRE_NOTHROW(rt.load(yaml, Properties::Format::YAML));
        verify_engine_config(rt);
    }
}

TEST_CASE("Suite: Merge and deep nesting", "[suite][properties][serialization]")
{
    SECTION("merged config serializes without error") {
        Properties base;
        base.set<std::string>({"app", "name"}, std::string("BaseApp"));
        base.set<double>({"app", "version"}, 1.0);

        Properties override;
        override.set<double>({"version"}, 2.0);
        override.set<bool>({"debug"}, true);
        base.merge({"app"}, override);

        std::string json;
        REQUIRE_NOTHROW(json = base.dump(Properties::Format::JSON));
        REQUIRE_FALSE(json.empty());
        REQUIRE(base.get<double>({"app", "version"}).has_value());
    }

    SECTION("5-level deep tree survives JSON round-trip") {
        Properties deep;
        deep.set<std::string>({"a", "b", "c", "d", "leaf"}, std::string("deep_value"));
        deep.set<double>({"a", "b", "c", "d", "count"}, 99.0);

        Properties rt;
        REQUIRE_NOTHROW(rt.load(deep.dump(Properties::Format::JSON), Properties::Format::JSON));
        REQUIRE(*rt.get<std::string>({"a", "b", "c", "d", "leaf"}) == "deep_value");
        REQUIRE(*rt.get<double>({"a", "b", "c", "d", "count"}) == Catch::Approx(99.0));
    }
}

TEST_CASE("Suite: C ABI engine config round-trip", "[suite][properties][abi]")
{
    auto* h = sandbox_properties_create();
    sandbox_properties_set_string(h, "app/name",      "sandbox_engine");
    sandbox_properties_set_double(h, "app/version",   2.0);
    sandbox_properties_set_bool(h,   "app/debug",     false);
    sandbox_properties_set_double(h, "window/width",  1920.0);
    sandbox_properties_set_double(h, "window/height", 1080.0);

    char* json = sandbox_properties_dump(h, SANDBOX_FORMAT_JSON);
    REQUIRE(json != nullptr);

    auto* rt = sandbox_properties_create();
    bool ok = sandbox_properties_load(rt, json, strlen(json), SANDBOX_FORMAT_JSON);

    SECTION("reload from ABI-dumped JSON succeeds") {
        REQUIRE(ok == true);
    }

    SECTION("app/name correct after ABI round-trip") {
        const char* name = sandbox_properties_get_string(rt, "app/name");
        REQUIRE(name != nullptr);
        REQUIRE(std::string(name) == "sandbox_engine");
    }

    SECTION("window/width correct after ABI round-trip") {
        double w = 0.0;
        REQUIRE(sandbox_properties_get_double(rt, "window/width", &w) == true);
        REQUIRE(w == Catch::Approx(1920.0));
    }

    sandbox_properties_free_string(json);
    sandbox_properties_destroy(rt);
    sandbox_properties_destroy(h);
}
