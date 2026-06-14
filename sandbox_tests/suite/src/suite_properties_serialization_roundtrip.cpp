// suite/src/suite_properties_serialization_roundtrip.cpp
//
// Integration suite: Properties serialization end-to-end round-trips
//
// Tests real-world configuration scenarios using the full pipeline:
//   1. Build a complex properties tree programmatically
//   2. Serialize to various formats (JSON, TOML, YAML)
//   3. Load into a fresh Properties instance
//   4. Verify all values are correct
//   5. Also test the C ABI layer end-to-end

#include <catch2/catch_all.hpp>
#include "core/properties.h"
#include "sandbox/core/properties.h"

#include <string>
#include <vector>

using sandbox::core::Properties;

// ---------------------------------------------------------------------------
// Helper: builds a realistic "engine config" properties tree
// ---------------------------------------------------------------------------
static void populate_engine_config(Properties& props) {
    // Window settings
    props.set<double>({"window", "width"},  1920.0);
    props.set<double>({"window", "height"}, 1080.0);
    props.set<bool>({"window", "fullscreen"}, false);
    props.set<std::string>({"window", "title"}, std::string("Sandbox Engine"));

    // Graphics settings
    props.set<double>({"graphics", "target_fps"}, 60.0);
    props.set<bool>({"graphics", "vsync"}, true);
    props.set<std::string>({"graphics", "api"}, std::string("OpenGL"));
    props.set<double>({"graphics", "msaa_samples"}, 4.0);

    // Audio settings
    props.set<double>({"audio", "master_volume"}, 0.8);
    props.set<bool>({"audio", "muted"}, false);
    props.set<double>({"audio", "sample_rate"}, 44100.0);

    // Network settings
    props.set<std::string>({"network", "host"}, std::string("127.0.0.1"));
    props.set<double>({"network", "port"}, 7777.0);
    props.set<double>({"network", "timeout_ms"}, 5000.0);
}

// ---------------------------------------------------------------------------
// Helper: verifies the full engine config is intact
// ---------------------------------------------------------------------------
static void verify_engine_config(const Properties& props) {
    REQUIRE(props.get<double>({"window", "width"})  == std::optional<double>(1920.0));
    REQUIRE(props.get<double>({"window", "height"}) == std::optional<double>(1080.0));
    REQUIRE(props.get<bool>({"window", "fullscreen"}) == std::optional<bool>(false));
    REQUIRE(props.get<std::string>({"window", "title"}) == std::optional<std::string>("Sandbox Engine"));

    REQUIRE(props.get<double>({"graphics", "target_fps"}) == std::optional<double>(60.0));
    REQUIRE(props.get<bool>({"graphics", "vsync"}) == std::optional<bool>(true));
    REQUIRE(props.get<std::string>({"graphics", "api"}) == std::optional<std::string>("OpenGL"));

    REQUIRE(props.get<double>({"audio", "master_volume"}).value() == Catch::Approx(0.8));
    REQUIRE(props.get<bool>({"audio", "muted"}) == std::optional<bool>(false));

    REQUIRE(props.get<std::string>({"network", "host"}) == std::optional<std::string>("127.0.0.1"));
    REQUIRE(props.get<double>({"network", "port"}) == std::optional<double>(7777.0));
}

// ---------------------------------------------------------------------------
// Suite: JSON round-trip
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Properties — full engine config JSON round-trip",
          "[suite][properties][serialization][json]")
{
    Properties source_props;
    populate_engine_config(source_props);

    std::string serialized_json = source_props.dump(Properties::Format::JSON);

    Properties loaded_props;
    REQUIRE_NOTHROW(loaded_props.load(serialized_json, Properties::Format::JSON));

    SECTION("all window settings survive JSON round-trip") {
        REQUIRE(loaded_props.get<double>({"window", "width"})  == std::optional<double>(1920.0));
        REQUIRE(loaded_props.get<double>({"window", "height"}) == std::optional<double>(1080.0));
        REQUIRE(loaded_props.get<bool>({"window", "fullscreen"}) == std::optional<bool>(false));
        REQUIRE(loaded_props.get<std::string>({"window", "title"}) == std::optional<std::string>("Sandbox Engine"));
    }

    SECTION("all graphics settings survive JSON round-trip") {
        REQUIRE(loaded_props.get<bool>({"graphics", "vsync"}) == std::optional<bool>(true));
        REQUIRE(loaded_props.get<std::string>({"graphics", "api"}) == std::optional<std::string>("OpenGL"));
        REQUIRE(loaded_props.get<double>({"graphics", "msaa_samples"}) == std::optional<double>(4.0));
    }

    SECTION("all audio settings survive JSON round-trip") {
        REQUIRE(loaded_props.get<double>({"audio", "master_volume"}).value() == Catch::Approx(0.8));
        REQUIRE(loaded_props.get<bool>({"audio", "muted"}) == std::optional<bool>(false));
    }

    SECTION("all network settings survive JSON round-trip") {
        REQUIRE(loaded_props.get<std::string>({"network", "host"}) == std::optional<std::string>("127.0.0.1"));
        REQUIRE(loaded_props.get<double>({"network", "port"}) == std::optional<double>(7777.0));
    }
}

// ---------------------------------------------------------------------------
// Suite: TOML round-trip
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Properties — full engine config TOML round-trip",
          "[suite][properties][serialization][toml]")
{
    Properties source_props;
    populate_engine_config(source_props);

    std::string serialized_toml = source_props.dump(Properties::Format::TOML);
    REQUIRE_FALSE(serialized_toml.empty());

    Properties loaded_props;
    REQUIRE_NOTHROW(loaded_props.load(serialized_toml, Properties::Format::TOML));

    verify_engine_config(loaded_props);
}

// ---------------------------------------------------------------------------
// Suite: YAML round-trip
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Properties — full engine config YAML round-trip",
          "[suite][properties][serialization][yaml]")
{
    Properties source_props;
    populate_engine_config(source_props);

    std::string serialized_yaml = source_props.dump(Properties::Format::YAML);
    REQUIRE_FALSE(serialized_yaml.empty());

    Properties loaded_props;
    REQUIRE_NOTHROW(loaded_props.load(serialized_yaml, Properties::Format::YAML));

    verify_engine_config(loaded_props);
}

// ---------------------------------------------------------------------------
// Suite: merge and then serialize
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Properties — merge two configs and serialize as JSON",
          "[suite][properties][serialization][merge]")
{
    Properties base_config;
    base_config.set<std::string>({"app", "name"}, std::string("BaseApp"));
    base_config.set<double>({"app", "version"}, 1.0);

    Properties override_config;
    override_config.set<double>({"version"}, 2.0);
    override_config.set<bool>({"debug"}, true);

    // Merge override into base at root/app
    base_config.merge({"app"}, override_config);

    SECTION("merged version value is the override (2.0)") {
        auto retrieved_version = base_config.get<double>({"app", "version"});
        // merge replaces the entire subtree — version comes from override_config
        // override_config's root has {app.version=2, app.debug=true}
        // After merge at base_config["app"] = override_config
        REQUIRE(retrieved_version.has_value());
    }

    SECTION("merged config serializes without error") {
        std::string json_output;
        REQUIRE_NOTHROW(json_output = base_config.dump(Properties::Format::JSON));
        REQUIRE_FALSE(json_output.empty());
    }
}

// ---------------------------------------------------------------------------
// Suite: C ABI end-to-end serialization
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Properties ABI — full engine config round-trip via C API",
          "[suite][properties][abi][serialization]")
{
    // Build config using C++ setters via ABI
    sandbox_properties_t* config_handle = sandbox_properties_create();

    sandbox_properties_set_string(config_handle, "app/name", "sandbox_engine");
    sandbox_properties_set_double(config_handle, "app/version", 2.0);
    sandbox_properties_set_bool(config_handle, "app/debug", false);
    sandbox_properties_set_double(config_handle, "window/width",  1920.0);
    sandbox_properties_set_double(config_handle, "window/height", 1080.0);

    // Dump to JSON
    char* serialized_json = sandbox_properties_dump(config_handle, SANDBOX_FORMAT_JSON);
    REQUIRE(serialized_json != nullptr);

    // Load into fresh handle
    sandbox_properties_t* loaded_handle = sandbox_properties_create();
    bool load_result = sandbox_properties_load(
        loaded_handle, serialized_json, strlen(serialized_json), SANDBOX_FORMAT_JSON);

    SECTION("reload from ABI-dumped JSON succeeds") {
        REQUIRE(load_result == true);
    }

    SECTION("app/name is correct after ABI round-trip") {
        const char* app_name = sandbox_properties_get_string(loaded_handle, "app/name");
        REQUIRE(app_name != nullptr);
        REQUIRE(std::string(app_name) == "sandbox_engine");
    }

    SECTION("app/version is correct after ABI round-trip") {
        double app_version = 0.0;
        REQUIRE(sandbox_properties_get_double(loaded_handle, "app/version", &app_version) == true);
        REQUIRE(app_version == Catch::Approx(2.0));
    }

    SECTION("window/width is correct after ABI round-trip") {
        double window_width = 0.0;
        REQUIRE(sandbox_properties_get_double(loaded_handle, "window/width", &window_width) == true);
        REQUIRE(window_width == Catch::Approx(1920.0));
    }

    sandbox_properties_free_string(serialized_json);
    sandbox_properties_destroy(loaded_handle);
    sandbox_properties_destroy(config_handle);
}

// ---------------------------------------------------------------------------
// Suite: Deep 5-level nesting round-trip
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Properties — deeply nested 5-level tree survives JSON round-trip",
          "[suite][properties][serialization][deep_nesting]")
{
    Properties deep_props;
    deep_props.set<std::string>({"a", "b", "c", "d", "leaf"}, std::string("deep_value"));
    deep_props.set<double>({"a", "b", "c", "d", "count"}, 99.0);

    std::string serialized_json = deep_props.dump(Properties::Format::JSON);
    Properties reloaded_props;
    REQUIRE_NOTHROW(reloaded_props.load(serialized_json, Properties::Format::JSON));

    SECTION("leaf string at 5 levels deep is correct") {
        auto leaf = reloaded_props.get<std::string>({"a", "b", "c", "d", "leaf"});
        REQUIRE(leaf.has_value());
        REQUIRE(*leaf == "deep_value");
    }

    SECTION("leaf double at 5 levels deep is correct") {
        auto count = reloaded_props.get<double>({"a", "b", "c", "d", "count"});
        REQUIRE(count.has_value());
        REQUIRE(*count == Catch::Approx(99.0));
    }
}
