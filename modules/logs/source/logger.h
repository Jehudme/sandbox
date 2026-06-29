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

        logger(flecs::world& world);
        ~logger();

        // Make logger move-constructible so it can be stored directly in Flecs
        logger(logger&&) = default;
        logger& operator=(logger&&) = default;

        // Prevent copying
        logger(const logger&) = delete;
        logger& operator=(const logger&) = delete;

        void log(level level, const char* message);

    private:
        std::unique_ptr<spdlog::logger> m_logger;
    };

}
