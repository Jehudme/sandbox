#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/events/logger.h"
#include "spdlog/spdlog.h"

namespace sandbox::modules {
    class logger {
    public:
        logger(world& ecs);
        ~logger();

    private:
        // Changed to const reference to match the event channel's subscription callback
        void log(const events::log& log_event);

        std::shared_ptr<spdlog::logger> m_logger;

        // Added to track whether the engine should automatically halt/throw on errors
        bool m_throw_on_error = false;
    };
}