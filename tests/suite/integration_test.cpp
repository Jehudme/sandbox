#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/bootstrapper.h"
#include <sandbox/api/logger_api.h>
#include "sandbox/event_bus/event_bus.h"
#include <flecs.h>

using namespace sandbox;

class MockLogger {
public:
    int log_count = 0;
    std::string last_msg;

    static int32_t log_cb(void* instance, const uint8_t* log_msg_fb, size_t size) {
        auto* self = static_cast<MockLogger*>(instance);
        self->log_count++;
        self->last_msg = "test"; // Simplified for raw ABI test
        return 0;
    }
    
    static void set_property_cb(void* instance, const char* key, const char* json_value) {}
    static int32_t get_property_cb(const void* instance, const char* key, sandbox_payload* out_payload) { return -1; }
};

struct mock_logger_module {
    mock_logger_module(flecs::world& ecs) {
        static MockLogger logger_instance;
        sandbox::logger_service svc;
        svc.instance = &logger_instance;
        svc.log = &MockLogger::log_cb;
        svc.set_property = &MockLogger::set_property_cb;
        svc.get_property = &MockLogger::get_property_cb;
        ecs.set<logger_service>(svc);
    }
};

struct dummy_module {
    dummy_module(flecs::world& ecs) {}
};

TEST_CASE("Event & Interface Mocking", "[integration]") {
    flecs::world ecs;
    bootstrapper boot(ecs);
    library_registry reg;

    SECTION("Mock Logger Locking") {
        // Register Mock as v9.9.9
        reg.services.push_back(create_service_info("logger", 9, 9));
        reg.modules.push_back(create_module_info<mock_logger_module>("mock_logger", 9, 9, 9, {}, "logger"));

        // Register a test module that requires logger v9.9
        reg.modules.push_back(create_module_info<dummy_module>("test_module", 1, 0, 0, {
            { requirement::kind::service, requirement::strictness::require, "logger", 9, 9 }
        }));

        boot.stage(reg);
        boot.activate("test_module");
        REQUIRE_NOTHROW(boot.execute(ecs));

        // Verify that the mock logger is accessible
        const auto* logger_svc = ecs.try_get<logger_service>();
        REQUIRE(logger_svc != nullptr);
        REQUIRE(logger_svc->instance != nullptr);
    }

    SECTION("Event Synchronous Reception") {
        reg.services.push_back(create_service_info("logger", 9, 9));
        reg.modules.push_back(create_module_info<mock_logger_module>("mock_logger", 9, 9, 9, {}, "logger"));

        boot.stage(reg);
        boot.activate("mock_logger");
        boot.execute(ecs);

        auto* svc = ecs.try_get_mut<logger_service>();
        MockLogger* mock_api = static_cast<MockLogger*>(svc->instance);
        
        mock_api->log_count = 0;

        // Fire event directly using the mock
        std::vector<uint8_t> dummy_fb(10, 0); // Fake flatbuffer data
        svc->log(svc->instance, dummy_fb.data(), dummy_fb.size());

        REQUIRE(mock_api->log_count == 1);
        REQUIRE(mock_api->last_msg == "test");
    }
}
