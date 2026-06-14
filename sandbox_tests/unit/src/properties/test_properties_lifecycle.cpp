// unit/src/properties/test_properties_lifecycle.cpp
//
// Unit tests for Properties class lifecycle:
//   - Default construction (produces an empty, valid object)
//   - Move construction / move assignment (via sub())
//   - Multiple independent instances do not share state

#include <catch2/catch_all.hpp>
#include "core/properties.h"  // internal header (source-private)

using sandbox::core::Properties;

// ---------------------------------------------------------------------------
// Feature: Properties — Default Construction
// ---------------------------------------------------------------------------
TEST_CASE("Prop: default construction valid empty obj",
          "[properties][lifecycle]")
{
    Properties properties;

    SECTION("has() returns false on any path for a freshly constructed object") {
        REQUIRE_FALSE(properties.has({"any_key"}));
        REQUIRE_FALSE(properties.has({"nested", "key"}));
    }

    SECTION("keys() returns empty vector on a freshly constructed object") {
        Properties::Keys returned_keys = properties.keys({});
        REQUIRE(returned_keys.empty());
    }

    SECTION("get() returns nullopt on a freshly constructed object") {
        REQUIRE_FALSE(properties.get<int64_t>({"missing"}).has_value());
        REQUIRE_FALSE(properties.get<double>({"missing"}).has_value());
        REQUIRE_FALSE(properties.get<bool>({"missing"}).has_value());
        REQUIRE_FALSE(properties.get<std::string>({"missing"}).has_value());
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Multiple Independent Instances
// ---------------------------------------------------------------------------
TEST_CASE("Prop: two instances do not share internal state",
          "[properties][lifecycle]")
{
    Properties first_instance;
    Properties second_instance;

    first_instance.set({"value"}, 42.0);

    SECTION("setting a value on one instance does not affect the other") {
        REQUIRE(first_instance.has({"value"}));
        REQUIRE_FALSE(second_instance.has({"value"}));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties — Sub-object independence
// ---------------------------------------------------------------------------
TEST_CASE("Prop: sub() returns an independent copy",
          "[properties][lifecycle]")
{
    Properties source_properties;
    source_properties.set({"config", "timeout"}, 30.0);

    Properties extracted_sub = source_properties.sub({"config"});

    SECTION("the extracted sub-properties has the key at the correct relative path") {
        REQUIRE(extracted_sub.has({"timeout"}));
        auto retrieved_value = extracted_sub.get<double>({"timeout"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(30.0));
    }

    SECTION("modifying the source does not affect the already-extracted sub") {
        source_properties.set({"config", "timeout"}, 999.0);

        // The extracted sub was a deep copy — it still has 30
        auto retrieved_value = extracted_sub.get<double>({"timeout"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(30.0));
    }

    SECTION("sub() on a missing path returns an empty Properties object") {
        Properties empty_sub = source_properties.sub({"nonexistent", "path"});
        REQUIRE_FALSE(empty_sub.has({"any_key"}));
    }
}
