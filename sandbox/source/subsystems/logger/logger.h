#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/utilities/properties.h"
#include "sandbox/subsystems/logger/ilogger.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace sandbox::modules {

    class logger : public ilogger {
    public:
        logger(world& ecs);
        ~logger() override;

        std::expected<void, std::string> log(const events::log& log_event) override;

        void set_property(const std::string& key, const std::any& value) override;
        std::any get_property(const std::string& key) const override;

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        bool m_throw_on_error{false};
    };

} // namespace sandbox::modules
