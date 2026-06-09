#include <catch2/catch_test_macros.hpp>
#include "sandbox/utilities/properties.h"
#include <string>
#include <glaze/glaze.hpp>

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

#include "sandbox/subsystems/logger/ilogger.h"
#include <cstdlib>

TEST_CASE("Properties serialization across C-ABI boundary", "[core][properties][abi]") {
    struct mock_logger : public sandbox::ilogger {
        std::string stored_json;

        int32_t log(const uint8_t* log_msg_fb, size_t size) override { return 0; }
        
        void set_property(const char* key, const char* json_value) override {
            if (std::string(key) == "test_prop") {
                stored_json = json_value;
            }
        }
        
        int32_t get_property(const char* key, sandbox_payload* out_payload) const override {
            if (std::string(key) == "test_prop") {
                uint8_t* ptr = static_cast<uint8_t*>(std::malloc(stored_json.size() + 1));
                std::memcpy(ptr, stored_json.c_str(), stored_json.size() + 1);
                out_payload->bytes = ptr;
                out_payload->size = stored_json.size();
                out_payload->free_func = [](void* p) { std::free(p); };
                return 0;
            }
            return -1;
        }
    } mock;

    SECTION("Serialize property, cross boundary via set_property, and retrieve via get_property") {
        std::string serialized;
        glz::write_json(42, serialized);
        
        mock.set_property("test_prop", serialized.c_str());
        
        sandbox_payload payload{};
        REQUIRE(mock.get_property("test_prop", &payload) == 0);
        REQUIRE(payload.bytes != nullptr);
        
        std::string retrieved_json(reinterpret_cast<const char*>(payload.bytes), payload.size);
        int retrieved_val = 0;
        REQUIRE(glz::read_json(retrieved_val, retrieved_json) == glz::error_code::none);
        REQUIRE(retrieved_val == 42);
        
        if (payload.free_func) payload.free_func(payload.bytes);
    }
}

