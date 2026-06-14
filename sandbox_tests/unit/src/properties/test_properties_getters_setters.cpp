// unit/src/properties/test_properties_getters_setters.cpp
// Tests for Properties typed get<T>() and set<T>() with all supported types.

#include <catch2/catch_all.hpp>
#include "core/properties.h"

using sandbox::core::Properties;

TEST_CASE("Prop: typed set/get round-trips", "[properties][getters_setters]")
{
    Properties p;

    SECTION("int64_t stores and retrieves") {
        p.set<int64_t>({"count"}, 42);
        REQUIRE(*p.get<int64_t>({"count"}) == 42);
        REQUIRE(*p.get<double>({"count"}) == Catch::Approx(42.0));
        REQUIRE_FALSE(p.get<bool>({"count"}).has_value());

        p.set<int64_t>({"large"}, 9'000'000);
        REQUIRE(*p.get<int64_t>({"large"}) == 9'000'000);

        p.set<int64_t>({"neg"}, -1024);
        REQUIRE(*p.get<int64_t>({"neg"}) == -1024);

        p.set<int64_t>({"zero"}, 0);
        REQUIRE(*p.get<int64_t>({"zero"}) == 0);
    }

    SECTION("double stores and retrieves") {
        p.set<double>({"pi"}, 3.14159);
        REQUIRE(*p.get<double>({"pi"}) == Catch::Approx(3.14159));

        p.set<double>({"neg"}, -0.001);
        REQUIRE(*p.get<double>({"neg"}) == Catch::Approx(-0.001));

        p.set<double>({"zero"}, 0.0);
        REQUIRE(*p.get<double>({"zero"}) == Catch::Approx(0.0));
    }

    SECTION("bool stores and retrieves") {
        p.set<bool>({"t"}, true);
        REQUIRE(*p.get<bool>({"t"}) == true);

        p.set<bool>({"f"}, false);
        REQUIRE(*p.get<bool>({"f"}) == false);

        // bool is not reinterpretable as numeric
        REQUIRE_FALSE(p.get<double>({"t"}).has_value());
        REQUIRE_FALSE(p.get<int64_t>({"t"}).has_value());
    }

    SECTION("string stores and retrieves") {
        p.set<std::string>({"greet"}, std::string{"hello world"});
        REQUIRE(*p.get<std::string>({"greet"}) == "hello world");

        p.set<std::string>({"empty"}, std::string{});
        REQUIRE(p.get<std::string>({"empty"})->empty());

        p.set<std::string>({"path"}, std::string{"path/to/resource"});
        REQUIRE(*p.get<std::string>({"path"}) == "path/to/resource");
    }
}

TEST_CASE("Prop: nested paths and overwrite", "[properties][getters_setters]")
{
    Properties p;

    SECTION("deep nested path (3 levels)") {
        p.set<double>({"a", "b", "c"}, 99.0);
        REQUIRE(*p.get<double>({"a", "b", "c"}) == Catch::Approx(99.0));
        REQUIRE(p.has({"a"}));
        REQUIRE(p.has({"a", "b"}));

        p.set<double>({"a", "b", "d"}, 100.0);
        REQUIRE(*p.get<double>({"a", "b", "c"}) == Catch::Approx(99.0));
        REQUIRE(*p.get<double>({"a", "b", "d"}) == Catch::Approx(100.0));
    }

    SECTION("overwrite with same type replaces value") {
        p.set<double>({"key"}, 1.0);
        p.set<double>({"key"}, 2.0);
        REQUIRE(*p.get<double>({"key"}) == Catch::Approx(2.0));
    }

    SECTION("overwrite with different type replaces value") {
        p.set<double>({"key"}, 1.0);
        p.set<bool>({"key"}, true);
        REQUIRE_FALSE(p.get<double>({"key"}).has_value());
        REQUIRE(*p.get<bool>({"key"}) == true);

        p.set<std::string>({"key"}, std::string{"replaced"});
        REQUIRE_FALSE(p.get<bool>({"key"}).has_value());
        REQUIRE(*p.get<std::string>({"key"}) == "replaced");
    }

    SECTION("get on missing key returns nullopt for all types") {
        p.set<double>({"existing"}, 5.0);
        REQUIRE_FALSE(p.get<double>({"nonexistent"}).has_value());
        REQUIRE_FALSE(p.get<int64_t>({"nonexistent"}).has_value());
        REQUIRE_FALSE(p.get<bool>({"nonexistent"}).has_value());
        REQUIRE_FALSE(p.get<std::string>({"nonexistent"}).has_value());
    }
}
