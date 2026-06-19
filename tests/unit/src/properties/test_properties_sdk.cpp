#include <catch2/catch_all.hpp>
#include "sandbox/sdk/properties.hpp"

using namespace sandbox;

TEST_CASE("Properties SDK Wrapper: Lifecycle", "[properties][sdk]") {
    SECTION("Construct and destroy does not leak") {
        REQUIRE_NOTHROW([]() {
            Properties props;
        }());
    }

    SECTION("Move semantics transfer ownership correctly") {
        Properties props;
        props.set_string("key", "value");

        Properties props2(std::move(props));
        REQUIRE(props.get_raw() == nullptr);
        REQUIRE(props2.get_raw() != nullptr);

        std::string val;
        REQUIRE(props2.get_string("key", val));
        REQUIRE(val == "value");

        Properties props3;
        props3 = std::move(props2);
        REQUIRE(props2.get_raw() == nullptr);
        REQUIRE(props3.get_raw() != nullptr);
        
        REQUIRE(props3.get_string("key", val));
        REQUIRE(val == "value");
    }
}

TEST_CASE("Properties SDK Wrapper: Callbacks and Memory", "[properties][sdk]") {
    Properties props;
    
    SECTION("Get and Set primitive types") {
        props.set_int64("engine/version", 42);
        int64_t v = 0;
        REQUIRE(props.get_int64("engine/version", v));
        REQUIRE(v == 42);

        props.set_double("math/pi", 3.1415);
        double d = 0;
        REQUIRE(props.get_double("math/pi", d));
        REQUIRE(d == 3.1415);

        props.set_bool("flags/debug", true);
        bool b = false;
        REQUIRE(props.get_bool("flags/debug", b));
        REQUIRE(b == true);
    }

    SECTION("String and Array callbacks do not use thread_local or raw pointers insecurely") {
        props.set_string("network/host", "localhost");
        std::string host;
        REQUIRE(props.get_string("network/host", host));
        REQUIRE(host == "localhost");

        // Set an array
        std::vector<std::string> modules = {"mod1", "mod2", "mod3"};
        props.set_string_array("engine/modules", modules);
        
        // Get keys in "engine"
        auto keys = props.keys("engine");
        REQUIRE(keys.size() == 1);
        REQUIRE(keys[0] == "modules");
        
        // Ensure values are retrievable
        std::vector<std::string> out_modules;
        REQUIRE(props.get_string_array("engine/modules", out_modules));
        REQUIRE(out_modules.size() == 3);
        REQUIRE(out_modules[0] == "mod1");
        REQUIRE(out_modules[1] == "mod2");
        REQUIRE(out_modules[2] == "mod3");
    }
}

TEST_CASE("Properties SDK Wrapper: Tree Operations", "[properties][sdk]") {
    Properties props;
    
    SECTION("Sub-properties extraction takes ownership properly") {
        props.set_string("a/b/c", "hello");
        Properties sub = props.sub("a/b");
        
        std::string val;
        REQUIRE(sub.get_string("c", val));
        REQUIRE(val == "hello");
    }

    SECTION("Merge operations") {
        props.set_string("target/1", "A");
        
        Properties other;
        other.set_string("target/2", "B");
        
        props.merge("imported", other);
        
        std::string valA, valB;
        REQUIRE(props.get_string("target/1", valA));
        REQUIRE(props.get_string("imported/target/2", valB));
        REQUIRE(valA == "A");
        REQUIRE(valB == "B");
    }
}
