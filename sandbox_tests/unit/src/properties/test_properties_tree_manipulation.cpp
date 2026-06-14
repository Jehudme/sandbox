// unit/src/properties/test_properties_tree_manipulation.cpp
//
// Unit tests for Properties tree manipulation methods:
//   - has() — shallow and deep, present and missing
//   - clear() — root clear, key-level clear, nested clear
//   - keys() — empty object, single key, multiple keys, nested
//   - merge() — merge at root, merge at path, overwrite semantics
//   - sub() — extract sub-tree, missing path, nested sub

#include <catch2/catch_all.hpp>
#include "core/properties.h"

using sandbox::core::Properties;

// ---------------------------------------------------------------------------
// Feature: Properties::has()
// ---------------------------------------------------------------------------
TEST_CASE("Properties::has: shallow path detection",
          "[properties][tree][has]")
{
    Properties properties;
    properties.set<double>({"alpha"}, 1.0);
    properties.set<std::string>({"beta"}, std::string{"hello"});

    SECTION("returns true for a present shallow key") {
        REQUIRE(properties.has({"alpha"}));
        REQUIRE(properties.has({"beta"}));
    }

    SECTION("returns false for a missing shallow key") {
        REQUIRE_FALSE(properties.has({"gamma"}));
    }

    SECTION("returns true for the root path (empty path points to root object)") {
        REQUIRE(properties.has({}));
    }
}

TEST_CASE("Properties::has: deep nested path detection",
          "[properties][tree][has]")
{
    Properties properties;
    properties.set<double>({"level1", "level2", "level3"}, 7.0);

    SECTION("returns true for exact deep path") {
        REQUIRE(properties.has({"level1", "level2", "level3"}));
    }

    SECTION("returns true for intermediate paths") {
        REQUIRE(properties.has({"level1"}));
        REQUIRE(properties.has({"level1", "level2"}));
    }

    SECTION("returns false for a sibling that was not set") {
        REQUIRE_FALSE(properties.has({"level1", "level2", "other"}));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties::clear()
// ---------------------------------------------------------------------------
TEST_CASE("Properties::clear: clearing a leaf key",
          "[properties][tree][clear]")
{
    Properties properties;
    properties.set<double>({"x"}, 10.0);
    properties.set<double>({"y"}, 20.0);

    SECTION("key is absent after clear") {
        properties.clear({"x"});
        REQUIRE_FALSE(properties.has({"x"}));
    }

    SECTION("sibling key is unaffected by clearing another key") {
        properties.clear({"x"});
        REQUIRE(properties.has({"y"}));
    }
}

TEST_CASE("Properties::clear: clearing an intermediate object node",
          "[properties][tree][clear]")
{
    Properties properties;
    properties.set<double>({"config", "timeout"}, 30.0);
    properties.set<std::string>({"config", "host"}, std::string{"localhost"});
    properties.set<double>({"other"}, 5.0);

    properties.clear({"config"});

    SECTION("cleared key is gone") {
        REQUIRE_FALSE(properties.has({"config"}));
        REQUIRE_FALSE(properties.has({"config", "timeout"}));
    }

    SECTION("sibling of cleared key is intact") {
        REQUIRE(properties.has({"other"}));
    }
}

TEST_CASE("Properties::clear: clearing the root (empty path) resets everything",
          "[properties][tree][clear]")
{
    Properties properties;
    properties.set<double>({"a"}, 1.0);
    properties.set<double>({"b"}, 2.0);

    properties.clear({});

    SECTION("no keys remain after root clear") {
        Properties::Keys keys_after_clear = properties.keys({});
        REQUIRE(keys_after_clear.empty());
    }

    SECTION("all top-level keys are absent") {
        REQUIRE_FALSE(properties.has({"a"}));
        REQUIRE_FALSE(properties.has({"b"}));
    }
}

TEST_CASE("Properties::clear: clearing a non-existent key is a no-op",
          "[properties][tree][clear]")
{
    Properties properties;
    properties.set<double>({"existing"}, 42.0);

    // Must not throw
    REQUIRE_NOTHROW(properties.clear({"nonexistent"}));

    SECTION("existing key is unaffected") {
        REQUIRE(properties.has({"existing"}));
    }
}

// ---------------------------------------------------------------------------
// Feature: Properties::keys()
// ---------------------------------------------------------------------------
TEST_CASE("Properties::keys: empty object returns empty vector",
          "[properties][tree][keys]")
{
    Properties properties;
    Properties::Keys returned_keys = properties.keys({});
    REQUIRE(returned_keys.empty());
}

TEST_CASE("Properties::keys: single-key object returns that key",
          "[properties][tree][keys]")
{
    Properties properties;
    properties.set<double>({"only_key"}, 1.0);

    Properties::Keys returned_keys = properties.keys({});

    SECTION("exactly one key returned") {
        REQUIRE(returned_keys.size() == 1);
    }

    SECTION("the key has the expected name") {
        REQUIRE(returned_keys[0] == "only_key");
    }
}

TEST_CASE("Properties::keys: multi-key object returns all keys",
          "[properties][tree][keys]")
{
    Properties properties;
    properties.set<double>({"apple"}, 1.0);
    properties.set<double>({"banana"}, 2.0);
    properties.set<double>({"cherry"}, 3.0);

    Properties::Keys returned_keys = properties.keys({});

    SECTION("returns three keys") {
        REQUIRE(returned_keys.size() == 3);
    }

    SECTION("all expected key names are present") {
        std::sort(returned_keys.begin(), returned_keys.end());
        REQUIRE(returned_keys[0] == "apple");
        REQUIRE(returned_keys[1] == "banana");
        REQUIRE(returned_keys[2] == "cherry");
    }
}

TEST_CASE("Properties::keys: keys at nested path",
          "[properties][tree][keys]")
{
    Properties properties;
    properties.set<double>({"section", "width"}, 100.0);
    properties.set<double>({"section", "height"}, 200.0);

    Properties::Keys nested_keys = properties.keys({"section"});

    SECTION("returns exactly two keys from nested object") {
        REQUIRE(nested_keys.size() == 2);
    }

    SECTION("keys contain the expected names") {
        std::sort(nested_keys.begin(), nested_keys.end());
        REQUIRE(nested_keys[0] == "height");
        REQUIRE(nested_keys[1] == "width");
    }
}

TEST_CASE("Properties::keys: path to a leaf returns empty vector",
          "[properties][tree][keys]")
{
    Properties properties;
    properties.set<double>({"leaf_node"}, 5.0);

    Properties::Keys returned_keys = properties.keys({"leaf_node"});
    REQUIRE(returned_keys.empty());
}

// ---------------------------------------------------------------------------
// Feature: Properties::merge()
// ---------------------------------------------------------------------------
TEST_CASE("Properties::merge: merging another Properties into a sub-path",
          "[properties][tree][merge]")
{
    Properties destination_properties;
    destination_properties.set<double>({"existing_key"}, 1.0);

    Properties source_properties;
    source_properties.set<double>({"merged_key"}, 2.0);
    source_properties.set<std::string>({"label"}, std::string{"source"});

    destination_properties.merge({"imported"}, source_properties);

    SECTION("merged content is accessible at the given path") {
        REQUIRE(destination_properties.has({"imported", "merged_key"}));
        REQUIRE(destination_properties.has({"imported", "label"}));
    }

    SECTION("merged numeric value is correct") {
        auto retrieved_value = destination_properties.get<double>({"imported", "merged_key"});
        REQUIRE(retrieved_value.has_value());
        REQUIRE(*retrieved_value == Catch::Approx(2.0));
    }

    SECTION("original key in destination is unaffected") {
        REQUIRE(destination_properties.has({"existing_key"}));
    }
}

TEST_CASE("Properties::merge: merging at root replaces root data",
          "[properties][tree][merge]")
{
    Properties destination_properties;
    destination_properties.set<double>({"old_key"}, 99.0);

    Properties replacement_properties;
    replacement_properties.set<double>({"new_key"}, 1.0);

    destination_properties.merge({}, replacement_properties);

    SECTION("old key is replaced") {
        REQUIRE_FALSE(destination_properties.has({"old_key"}));
    }

    SECTION("new key is present") {
        REQUIRE(destination_properties.has({"new_key"}));
    }
}
