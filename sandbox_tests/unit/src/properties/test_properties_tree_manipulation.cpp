// unit/src/properties/test_properties_tree_manipulation.cpp
// Tests for has(), clear(), keys(), and merge().

#include <catch2/catch_all.hpp>
#include "core/properties.h"

using sandbox::core::Properties;

TEST_CASE("Prop: has() detects paths", "[properties][tree][has]")
{
    Properties p;
    p.set<double>({"alpha"}, 1.0);
    p.set<std::string>({"beta"}, std::string{"hello"});
    p.set<double>({"level1", "level2", "level3"}, 7.0);

    SECTION("shallow present and missing keys") {
        REQUIRE(p.has({"alpha"}));
        REQUIRE(p.has({"beta"}));
        REQUIRE_FALSE(p.has({"gamma"}));
        REQUIRE(p.has({}));  // root always present when non-empty
    }

    SECTION("deep path — present and siblings") {
        REQUIRE(p.has({"level1", "level2", "level3"}));
        REQUIRE(p.has({"level1"}));
        REQUIRE(p.has({"level1", "level2"}));
        REQUIRE_FALSE(p.has({"level1", "level2", "other"}));
    }
}

TEST_CASE("Prop: clear() removes keys", "[properties][tree][clear]")
{
    SECTION("clearing a leaf removes only that key") {
        Properties p;
        p.set<double>({"x"}, 10.0);
        p.set<double>({"y"}, 20.0);
        p.clear({"x"});
        REQUIRE_FALSE(p.has({"x"}));
        REQUIRE(p.has({"y"}));
    }

    SECTION("clearing an intermediate node removes all children") {
        Properties p;
        p.set<double>({"config", "timeout"}, 30.0);
        p.set<std::string>({"config", "host"}, std::string{"localhost"});
        p.set<double>({"other"}, 5.0);
        p.clear({"config"});
        REQUIRE_FALSE(p.has({"config"}));
        REQUIRE_FALSE(p.has({"config", "timeout"}));
        REQUIRE(p.has({"other"}));
    }

    SECTION("clearing root (empty path) resets everything") {
        Properties p;
        p.set<double>({"a"}, 1.0);
        p.set<double>({"b"}, 2.0);
        p.clear({});
        REQUIRE(p.keys({}).empty());
        REQUIRE_FALSE(p.has({"a"}));
        REQUIRE_FALSE(p.has({"b"}));
    }

    SECTION("clearing nonexistent key is a no-op") {
        Properties p;
        p.set<double>({"existing"}, 42.0);
        REQUIRE_NOTHROW(p.clear({"nonexistent"}));
        REQUIRE(p.has({"existing"}));
    }
}

TEST_CASE("Prop: keys() enumerates children", "[properties][tree][keys]")
{
    SECTION("empty object returns empty") {
        Properties p;
        REQUIRE(p.keys({}).empty());
    }

    SECTION("single key object") {
        Properties p;
        p.set<double>({"only_key"}, 1.0);
        auto keys = p.keys({});
        REQUIRE(keys.size() == 1);
        REQUIRE(keys[0] == "only_key");
    }

    SECTION("multi-key object returns all keys") {
        Properties p;
        p.set<double>({"apple"}, 1.0);
        p.set<double>({"banana"}, 2.0);
        p.set<double>({"cherry"}, 3.0);
        auto keys = p.keys({});
        std::sort(keys.begin(), keys.end());
        REQUIRE(keys.size() == 3);
        REQUIRE(keys[0] == "apple");
        REQUIRE(keys[1] == "banana");
        REQUIRE(keys[2] == "cherry");
    }

    SECTION("nested path returns child keys") {
        Properties p;
        p.set<double>({"section", "width"}, 100.0);
        p.set<double>({"section", "height"}, 200.0);
        auto keys = p.keys({"section"});
        std::sort(keys.begin(), keys.end());
        REQUIRE(keys.size() == 2);
        REQUIRE(keys[0] == "height");
        REQUIRE(keys[1] == "width");
    }

    SECTION("path to a leaf returns empty") {
        Properties p;
        p.set<double>({"leaf"}, 5.0);
        REQUIRE(p.keys({"leaf"}).empty());
    }
}

TEST_CASE("Prop: merge() combines objects", "[properties][tree][merge]")
{
    SECTION("merge at sub-path") {
        Properties dst;
        dst.set<double>({"existing"}, 1.0);

        Properties src;
        src.set<double>({"merged_key"}, 2.0);
        src.set<std::string>({"label"}, std::string{"source"});

        dst.merge({"imported"}, src);

        REQUIRE(dst.has({"imported", "merged_key"}));
        REQUIRE(dst.has({"imported", "label"}));
        REQUIRE(*dst.get<double>({"imported", "merged_key"}) == Catch::Approx(2.0));
        REQUIRE(dst.has({"existing"}));
    }

    SECTION("merge at root replaces all keys") {
        Properties dst;
        dst.set<double>({"old"}, 99.0);

        Properties src;
        src.set<double>({"new"}, 1.0);

        dst.merge({}, src);

        REQUIRE_FALSE(dst.has({"old"}));
        REQUIRE(dst.has({"new"}));
    }
}
