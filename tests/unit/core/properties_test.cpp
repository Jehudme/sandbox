#include <catch2/catch_test_macros.hpp>
#include "sandbox/utilities/properties.h"
#include <string>

using namespace sandbox;

TEST_CASE("Properties Utility parsing and basic access", "[core][properties]") {
    SECTION("Parse valid JSON strings and raw byte arrays successfully") {
        std::string json = R"({"key": "value"})";
        auto props = properties::parse(json);
        REQUIRE(props.has_value());
        
        std::vector<std::byte> bytes;
        for (char c : json) bytes.push_back(static_cast<std::byte>(c));
        auto props_bytes = properties::parse(bytes);
        REQUIRE(props_bytes.has_value());
    }

    SECTION("Correctly extract nested keys using key_path") {
        auto props = properties::parse(R"({"nested": {"key": 42}})").value();
        auto val = props.get<int>({"nested", "key"});
        REQUIRE(val.has_value());
        REQUIRE(val.value() == 42);
    }

    SECTION("Return std::unexpected when querying a non-existent key") {
        properties props;
        auto val = props.get<int>({"missing"});
        REQUIRE_FALSE(val.has_value());
    }

    SECTION("Validate deep_merge overrides overlapping keys but preserves distinct ones") {
        auto base = properties::parse(R"({"a": 1, "b": {"c": 2}})").value();
        auto over = properties::parse(R"({"b": {"c": 3, "d": 4}})").value();
        base.merge(over);
        REQUIRE(base.get<int>({"a"}).value() == 1);
        REQUIRE(base.get<int>({"b", "c"}).value() == 3);
        REQUIRE(base.get<int>({"b", "d"}).value() == 4);
    }

    SECTION("Validate rename, move, and remove operations mutate the tree safely") {
        auto props = properties::parse(R"({"root": {"old_name": 10, "move_me": 20, "delete_me": 30}})").value();
        
        REQUIRE(props.rename({"root", "old_name"}, "new_name").has_value());
        REQUIRE(props.get<int>({"root", "new_name"}).value() == 10);
        
        REQUIRE(props.move({"root", "move_me"}, {"root", "moved"}).has_value());
        REQUIRE(props.get<int>({"root", "moved"}).value() == 20);
        
        REQUIRE(props.remove({"root", "delete_me"}).has_value());
        REQUIRE_FALSE(props.get<int>({"root", "delete_me"}).has_value());
    }
}
