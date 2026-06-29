#include "logger.h"

#include <string>
#include <vector>
#include <sandbox/sdk/configuration.hpp>

// spdlog includes
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h> // Required for rotating files

namespace sandbox::modules {

    logger::logger(flecs::world& world) {
        // ==========================================
        // 1. FETCH ALL CONFIGURATION ARGUMENTS
        // ==========================================

        // General
        std::optional<std::string> name_opt         = configuration::get<std::string>(world, "logs/name");
        std::optional<std::string> level_opt        = configuration::get<std::string>(world, "logs/level");
        std::optional<std::string> flush_level_opt  = configuration::get<std::string>(world, "logs/flush_level");
        std::optional<std::string> pattern_opt      = configuration::get<std::string>(world, "logs/pattern");

        // Console Sink
        std::optional<bool> console_opt             = configuration::get<bool>(world, "logs/console/enabled");

        // File Sink
        std::optional<std::string> file_opt         = configuration::get<std::string>(world, "logs/file/path");
        std::optional<bool> truncate_opt            = configuration::get<bool>(world, "logs/file/truncate");
        std::optional<bool> rotating_opt            = configuration::get<bool>(world, "logs/file/rotating");
        std::optional<int64_t> max_size_opt         = configuration::get<int64_t>(world, "logs/file/max_size");
        std::optional<int64_t> max_files_opt        = configuration::get<int64_t>(world, "logs/file/max_files");

        // Async Configuration
        std::optional<bool> async_opt               = configuration::get<bool>(world, "logs/async/enabled");
        std::optional<int64_t> queue_size_opt       = configuration::get<int64_t>(world, "logs/async/queue_size");
        std::optional<int64_t> thread_count_opt     = configuration::get<int64_t>(world, "logs/async/thread_count");

        // ==========================================
        // 2. CONSTRUCT SINKS
        // ==========================================
        std::vector<spdlog::sink_ptr> sinks;

        // Setup Console Sink (Defaults to true if omitted)
        if (console_opt.value_or(true)) {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        // Setup File Sink (Basic or Rotating)
        if (file_opt.has_value() && !file_opt.value().empty()) {
            std::string filepath = file_opt.value();

            if (rotating_opt.value_or(false)) {
                // Default to 5MB max size and 3 rotated files if omitted
                size_t max_size  = static_cast<size_t>(max_size_opt.value_or(1024 * 1024 * 5));
                size_t max_files = static_cast<size_t>(max_files_opt.value_or(3));
                sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filepath, max_size, max_files));
            } else {
                bool truncate = truncate_opt.value_or(true);
                sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(filepath, truncate));
            }
        }

        // ==========================================
        // 3. INITIALIZE LOGGER
        // ==========================================
        std::string logger_name = name_opt.value_or("sandbox");

        if (async_opt.value_or(false)) {
            // Default to queue size 8192 and 1 backing thread if omitted
            size_t queue_size   = static_cast<size_t>(queue_size_opt.value_or(8192));
            size_t thread_count = static_cast<size_t>(thread_count_opt.value_or(1));

            spdlog::init_thread_pool(queue_size, thread_count);
            m_logger = std::make_unique<spdlog::async_logger>(
                logger_name,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
        } else {
            m_logger = std::make_unique<spdlog::logger>(
                logger_name,
                sinks.begin(),
                sinks.end()
            );
        }

        // ==========================================
        // 4. APPLY SETTINGS (Pattern, Levels)
        // ==========================================

        // Apply custom formatting pattern if provided
        if (pattern_opt.has_value() && !pattern_opt.value().empty()) {
            m_logger->set_pattern(pattern_opt.value());
        }

        // Helper lambda to safely parse spdlog strings to enums
        auto parse_level = [](const std::string& lvl_str, spdlog::level::level_enum fallback) {
            spdlog::level::level_enum parsed = spdlog::level::from_str(lvl_str);
            // Protect against silent failures when users mistype the log level
            return (parsed == spdlog::level::off && lvl_str != "off") ? fallback : parsed;
        };

        // Apply Logger levels
        spdlog::level::level_enum active_lvl = parse_level(level_opt.value_or("info"), spdlog::level::info);
        spdlog::level::level_enum flush_lvl  = parse_level(flush_level_opt.value_or("warn"), spdlog::level::warn);

        m_logger->set_level(active_lvl);
        m_logger->flush_on(flush_lvl);
    }

    logger::~logger() {
        if (m_logger) {
            m_logger->flush();
        }
    }

    void logger::log(level lvl, const char* message) {
        if (!m_logger) return;

        spdlog::level::level_enum spd_lvl;
        switch (lvl) {
            case level::TRACE: spd_lvl = spdlog::level::trace; break;
            case level::DEBUG: spd_lvl = spdlog::level::debug; break;
            case level::INFO:  spd_lvl = spdlog::level::info;  break;
            case level::WARN:  spd_lvl = spdlog::level::warn;  break;
            case level::ERROR: spd_lvl = spdlog::level::err;   break;
            default:           spd_lvl = spdlog::level::info;  break;
        }

        m_logger->log(spd_lvl, message);
    }

}