#include <catch2/catch_all.hpp>
#include "sandbox/sdk/properties.hpp"

using namespace sandbox;

TEST_CASE("properties SDK Wrapper: Lifecycle", "[properties][sdk]") {
    SECTION("Construct and destroy does not leak") {
        REQUIRE_NOTHROW([]() {
            properties props;
        }());
    }

    SECTION("Move semantics transfer ownership correctly") {
        properties props;
        props.set("key", "value");

        properties props2(std::move(props));
        REQUIRE(props.get_raw() == nullptr);
        REQUIRE(props2.get_raw() != nullptr);

        std::string val;
        REQUIRE(props2.get("key", val));
        REQUIRE(val == "value");

        properties props3;
        props3 = std::move(props2);
        REQUIRE(props2.get_raw() == nullptr);
        REQUIRE(props3.get_raw() != nullptr);
        
        REQUIRE(props3.get("key", val));
        REQUIRE(val == "value");
    }
}

TEST_CASE("properties SDK Wrapper: Callbacks and Memory", "[properties][sdk]") {
    properties props;
    
    SECTION("Get and Set primitive types") {
        props.set("engine/version", 42LL);
        int64_t v = 0;
        REQUIRE(props.get("engine/version", v));
        REQUIRE(v == 42);

        props.set("math/pi", 3.1415);
        double d = 0.0;
        REQUIRE(props.get("math/pi", d));
        REQUIRE(d == Catch::Approx(3.1415));

        props.set("flags/debug", true);
        bool b = false;
        REQUIRE(props.get("flags/debug", b));
        REQUIRE(b == true);
    }

    SECTION("Get and Set string") {
        props.set("network/host", "localhost");
        std::string host;
        REQUIRE(props.get("network/host", host));
        REQUIRE(host == "localhost");
    }

    SECTION("Get and Set string array") {
        std::vector<std::string> modules = {"render", "physics", "audio"};
        props.set_array("engine/sandbox", modules);
        
        std::vector<std::string> out;
        REQUIRE(props.get_array("engine/sandbox", out));
        REQUIRE(out.size() == 3);
        REQUIRE(out[0] == "render");
        REQUIRE(out[2] == "audio");
    }
}

TEST_CASE("properties SDK Wrapper: Tree Operations", "[properties][sdk]") {
    properties props;
    
    SECTION("Sub-properties extraction takes ownership properly") {
        props.set("a/b/c", "hello");
        properties sub = props.sub("a/b");
        
        std::string val;
        REQUIRE(sub.get("c", val));
        REQUIRE(val == "hello");
    }

    SECTION("Merge operations") {
        props.set("target/1", "A");
        
        properties other;
        other.set("target/2", "B");
        
        props.merge("imported", other);
        
        std::string valA, valB;
        REQUIRE(props.get("target/1", valA));
        REQUIRE(props.get("imported/target/2", valB));
        REQUIRE(valA == "A");
        REQUIRE(valB == "B");
    }
}
