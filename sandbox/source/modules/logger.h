#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/events/logger.h"
#include "sandbox/utilities/properties.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace sandbox::modules {

    class logger {
    public:
        logger(world& ecs);
        ~logger();

    private:
        void log(const events::log& log_event);

        std::shared_ptr<spdlog::logger> m_logger;
        bool m_throw_on_error{false};
    };

} // namespace sandbox::modules