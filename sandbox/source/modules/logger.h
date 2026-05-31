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
        void log(const events::log& log_event);

        std::shared_ptr<spdlog::logger> m_logger;
    };
}
