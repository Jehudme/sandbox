#pragma once

#include <memory>

#include "flecs.h"

namespace spdlog {
    class logger;
}

namespace sandbox::modules {
    /**
     * @brief A global logger module that wraps spdlog and stores its state in the flecs world.
     */
    class logger_t {
    public:
        /**
         * @brief Defines the severity level of a log message.
         */
        enum class level_t : uint8_t {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR
        };

        /**
         * @brief Constructs a new logger and initializes it from the configuration.
         * @param entity_world The flecs world used to query configuration properties.
         */
        logger_t(flecs::world& entity_world);

        /**
         * @brief Destroys the logger instance.
         */
        ~logger_t();

        // Make logger move-constructible so it can be stored directly in Flecs
        logger_t(logger_t&&) = default;
        logger_t& operator=(logger_t&&) = default;

        // Prevent copying
        logger_t(const logger_t&) = delete;
        logger_t& operator=(const logger_t&) = delete;

        /**
         * @brief Submits a log message at the specified severity level.
         * @param log_level The severity level.
         * @param message The log message payload.
         */
        void log(level_t log_level, const char* message);

    private:
        std::unique_ptr<spdlog::logger> m_logger;
    };

}
