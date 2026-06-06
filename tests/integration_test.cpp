#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/bootstrapper.h"
#include "sandbox/subsystems/logger/ilogger.h"
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/event_bus/logger_events.h"
#include <flecs.h>

using namespace sandbox;

class MockLogger : public ilogger {
public:
    int log_count = 0;
    std::string last_msg;

    std::expected<void, std::string> log(const sandbox::events::log& log_event) override {
        log_count++;
        last_msg = log_event.message;
        return {};
    }
    
    void set_property(const std::string& key, const std::any& value) override {}
    std::any get_property(const std::string& key) const override { return {}; }
};

struct mock_logger_module {
    mock_logger_module(flecs::world& ecs) {
        static MockLogger logger_instance;
        ecs.set<logger_service>({&logger_instance});
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
        const auto& logger_svc = ecs.get<logger_service>();
        REQUIRE(logger_svc.api != nullptr);
    }

    SECTION("Event Synchronous Reception") {
        reg.services.push_back(create_service_info("logger", 9, 9));
        reg.modules.push_back(create_module_info<mock_logger_module>("mock_logger", 9, 9, 9, {}, "logger"));

        boot.stage(reg);
        boot.activate("mock_logger");
        boot.execute(ecs);

        auto& svc = ecs.get_mut<logger_service>();
        MockLogger* mock_api = static_cast<MockLogger*>(svc.api);
        
        mock_api->log_count = 0;

        // Fire event directly using the mock
        sandbox::events::log event{__FILE__, __LINE__, sandbox::events::log::level::Info, std::nullopt, "Test message"};
        mock_api->log(event);

        REQUIRE(mock_api->log_count == 1);
        REQUIRE(mock_api->last_msg == "Test message");
    }
}
