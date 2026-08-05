#include "logs_module.h"
#include <sandbox/sdk/logs.hpp>

#include <sandbox/sdk/configuration.hpp>
#include <string>
#include <vector>

// spdlog includes
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h> // Required for rotating files
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace sandbox::modules {

// TODO: cli_parser support all of these options
logger_t::logger_t(flecs::world &entity_world) {
  // ==========================================
  // 1. FETCH ALL CONFIGURATION ARGUMENTS
  // ==========================================

  // General
  std::optional<std::string> name_opt = configuration::get<std::string>(
      entity_world, "booting-configuration/logs-name");
  std::optional<std::string> level_opt = configuration::get<std::string>(
      entity_world, "booting-configuration/logs-level");
  std::optional<std::string> flush_level_opt = configuration::get<std::string>(
      entity_world, "booting-configuration/logs-flush_level");
  std::optional<std::string> pattern_opt = configuration::get<std::string>(
      entity_world, "booting-configuration/logs-pattern");

  // Console Sink
  std::optional<bool> console_opt = configuration::get<bool>(
      entity_world, "booting-configuration/logs-console_enabled");

  // File Sink
  std::optional<std::string> file_opt = configuration::get<std::string>(
      entity_world, "booting-configuration/logs-filepath");
  std::optional<bool> truncate_opt = configuration::get<bool>(
      entity_world, "booting-configuration/logs-file_truncate");
  std::optional<bool> rotating_opt = configuration::get<bool>(
      entity_world, "booting-configuration/logs-file_rotating");
  std::optional<int64_t> max_size_opt = configuration::get<int64_t>(
      entity_world, "booting-configuration/logs-file_max_size");
  std::optional<int64_t> max_files_opt = configuration::get<int64_t>(
      entity_world, "booting-configuration/logs-file_max_files");

  // Async Configuration
  std::optional<bool> async_opt = configuration::get<bool>(
      entity_world, "booting-configuration/logs-async_enabled");
  std::optional<int64_t> queue_size_opt = configuration::get<int64_t>(
      entity_world, "booting-configuration/logs-async_queue_size");
  std::optional<int64_t> thread_count_opt = configuration::get<int64_t>(
      entity_world, "booting-configuration/logs-async_thread_count");

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
      size_t max_size =
          static_cast<size_t>(max_size_opt.value_or(1024 * 1024 * 5));
      size_t max_files = static_cast<size_t>(max_files_opt.value_or(3));
      sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          filepath, max_size, max_files));
    } else {
      bool truncate = truncate_opt.value_or(true);
      sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          filepath, truncate));
    }
  }

  // ==========================================
  // 3. INITIALIZE LOGGER
  // ==========================================
  std::string logger_name = name_opt.value_or("sandbox");

  if (async_opt.value_or(false)) {
    // Default to queue size 8192 and 1 backing thread if omitted
    size_t queue_size = static_cast<size_t>(queue_size_opt.value_or(8192));
    size_t thread_count = static_cast<size_t>(thread_count_opt.value_or(1));

    spdlog::init_thread_pool(queue_size, thread_count);
    m_logger = std::make_unique<spdlog::async_logger>(
        logger_name, sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
  } else {
    m_logger = std::make_unique<spdlog::logger>(logger_name, sinks.begin(),
                                                sinks.end());
  }

  // ==========================================
  // 4. APPLY SETTINGS (Pattern, Levels)
  // ==========================================

  // Apply custom formatting pattern if provided
  if (pattern_opt.has_value() && !pattern_opt.value().empty()) {
    m_logger->set_pattern(pattern_opt.value());
  }

  // Helper lambda to safely parse spdlog strings to enums
  auto parse_level = [](const std::string &lvl_str,
                        spdlog::level::level_enum fallback) {
    spdlog::level::level_enum parsed = spdlog::level::from_str(lvl_str);
    // Protect against silent failures when users mistype the log level
    return (parsed == spdlog::level::off && lvl_str != "off") ? fallback
                                                              : parsed;
  };

  // Apply Logger levels
  spdlog::level::level_enum active_lvl =
      parse_level(level_opt.value_or("info"), spdlog::level::info);
  spdlog::level::level_enum flush_lvl =
      parse_level(flush_level_opt.value_or("warn"), spdlog::level::warn);

  m_logger->set_level(active_lvl);
  m_logger->flush_on(flush_lvl);
}

logger_t::~logger_t() {
  if (m_logger) {
    m_logger->flush();
  }
}

void logger_t::log(level_t log_level, const char *message) {
  if (!m_logger)
    return;

  spdlog::level::level_enum spdlog_level;
  switch (log_level) {
  case level_t::TRACE:
    spdlog_level = spdlog::level::trace;
    break;
  case level_t::DEBUG:
    spdlog_level = spdlog::level::debug;
    break;
  case level_t::INFO:
    spdlog_level = spdlog::level::info;
    break;
  case level_t::WARN:
    spdlog_level = spdlog::level::warn;
    break;
  case level_t::ERROR:
    spdlog_level = spdlog::level::err;
    break;
  default:
    spdlog_level = spdlog::level::info;
    break;
  }

  m_logger->log(spdlog_level, message);
}

} // namespace sandbox::modules

// ==========================================
static sandbox_requirement_info_t logs_requirements[] = {
    {.kind = SANDBOX_REQUIREMENT_KIND_SERVICE,
     .strictness = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
     .name = "configuration",
     .architecture = "sandbox",
     .version_major = 1,
     .version_minor = 0,
     .version_patch = -1}};

namespace sandbox::modules {
SANDBOX_DECLARE_MODULE(logger_t, {.name = "logs",
                                  .description = "Global logging module",
                                  .architecture = "sandbox",
                                  .version_major = 1,
                                  .version_minor = 0,
                                  .version_patch = 0,
                                  .service = &sandbox_logs_service_t_info,
                                  .requirements = logs_requirements,
                                  .requirement_count = 1})
}
