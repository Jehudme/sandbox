#pragma once

#include <memory>

#include "flecs.h"

namespace spdlog {
    class logger;
}

namespace sandbox::modules {
    class logger {
    public:
        enum class level : uint8_t {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR
        };

        logger(ecs_world_t* ecs);
        ~logger();

        void log(level level, const char* message);

    private:
        std::unique_ptr<spdlog::logger> m_logger;
    };

}
