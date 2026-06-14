// unit/src/properties/test_properties_lifecycle.cpp
// Tests for Properties construction, instance isolation, and sub().

#include <catch2/catch_all.hpp>
#include "core/properties.h"

using sandbox::core::Properties;

TEST_CASE("Prop: construction and isolation", "[properties][lifecycle]")
{
    SECTION("default-constructed object is empty") {
        Properties p;
        REQUIRE_FALSE(p.has({"any_key"}));
        REQUIRE_FALSE(p.has({"nested", "key"}));
        REQUIRE(p.keys({}).empty());
        REQUIRE_FALSE(p.get<int64_t>({"missing"}).has_value());
        REQUIRE_FALSE(p.get<double>({"missing"}).has_value());
        REQUIRE_FALSE(p.get<bool>({"missing"}).has_value());
        REQUIRE_FALSE(p.get<std::string>({"missing"}).has_value());
    }

    SECTION("two instances do not share state") {
        Properties a, b;
        a.set({"value"}, 42.0);
        REQUIRE(a.has({"value"}));
        REQUIRE_FALSE(b.has({"value"}));
    }
}

TEST_CASE("Prop: sub() is an independent copy", "[properties][lifecycle]")
{
    Properties source;
    source.set({"config", "timeout"}, 30.0);
    Properties sub = source.sub({"config"});

    SECTION("sub has key at relative path") {
        REQUIRE(sub.has({"timeout"}));
        REQUIRE(*sub.get<double>({"timeout"}) == Catch::Approx(30.0));
    }

    SECTION("modifying source does not affect sub") {
        source.set({"config", "timeout"}, 999.0);
        REQUIRE(*sub.get<double>({"timeout"}) == Catch::Approx(30.0));
    }

    SECTION("sub on missing path returns empty") {
        Properties empty = source.sub({"nonexistent", "path"});
        REQUIRE_FALSE(empty.has({"any_key"}));
    }
}
