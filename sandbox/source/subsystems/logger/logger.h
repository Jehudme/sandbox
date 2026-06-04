#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/utilities/properties.h"
#include "sandbox/subsystems/logger/ilogger.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace sandbox::modules {

    struct logger_config {
        std::string logger_name{"sandbox_core"};
        spdlog::level::level_enum boot_level{spdlog::level::info};
        bool throw_on_error{true};
    };

    class logger : public ilogger {
    public:
        logger(world& ecs, const logger_config& config = logger_config{});
        ~logger() override;

        std::expected<void, std::string> log(const events::log& log_event) override;

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        bool m_throw_on_error{false};
    };

} // namespace sandbox::modules
