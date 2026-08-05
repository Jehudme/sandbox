#include "cli_parser.h"
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace sandbox::launcher {

std::optional<sandbox::properties> parse_cli(int argc, char **argv) {
  CLI::App cli_app{"Sandbox Engine Launcher"};

  std::string mount_path;
  cli_app
      .add_option("target", mount_path,
                  "Path to the application to run (folder or zip)")
      ->required();

  bool clean_mode = false;
  cli_app.add_flag("--clean", clean_mode,
                   "Do not include default libraries and modules");

  bool dev_mode = false;
  cli_app.add_flag("--dev", dev_mode, "Enable developer mode");

  std::optional<std::string> logs_name;
  cli_app.add_option("--logs-name", logs_name, "Set logger name");

  std::optional<std::string> logs_level;
  cli_app.add_option(
      "--logs-level", logs_level,
      "Set log level (e.g., trace, debug, info, warn, error, critical)");

  std::optional<std::string> logs_flush_level;
  cli_app.add_option("--logs-flush-level", logs_flush_level, "Set flush level");

  std::optional<std::string> logs_pattern;
  cli_app.add_option("--logs-pattern", logs_pattern, "Set log pattern");

  std::optional<bool> logs_console;
  cli_app.add_flag("--logs-console,!--no-logs-console", logs_console,
                   "Enable/disable console logging");

  std::optional<std::string> logs_filepath;
  cli_app.add_option("--logs-filepath", logs_filepath, "Set log file path");

  std::optional<bool> logs_file_truncate;
  cli_app.add_flag("--logs-file-truncate,!--no-logs-file-truncate",
                   logs_file_truncate, "Enable/disable file truncation");

  std::optional<bool> logs_file_rotating;
  cli_app.add_flag("--logs-file-rotating,!--no-logs-file-rotating",
                   logs_file_rotating, "Enable/disable file rotation");

  std::optional<int64_t> logs_file_max_size;
  cli_app.add_option("--logs-file-max-size", logs_file_max_size,
                     "Set rotating file max size");

  std::optional<int64_t> logs_file_max_files;
  cli_app.add_option("--logs-file-max-files", logs_file_max_files,
                     "Set rotating max files");

  std::optional<bool> logs_async;
  cli_app.add_flag("--logs-async,!--no-logs-async", logs_async,
                   "Enable/disable async logging");

  std::optional<int64_t> logs_async_queue_size;
  cli_app.add_option("--logs-async-queue-size", logs_async_queue_size,
                     "Set async queue size");

  std::optional<int64_t> logs_async_thread_count;
  cli_app.add_option("--logs-async-thread-count", logs_async_thread_count,
                     "Set async thread count");

  std::vector<std::string> additional_libraries;
  cli_app.add_option("--lib", additional_libraries,
                     "Additional library paths to load");

  std::vector<std::string> additional_modules;
  cli_app.add_option(
      "--module", additional_modules,
      "Additional modules to activate (format: architecture-name@version)");

  std::vector<std::string> additional_arguments;
  cli_app.add_option(
      "--args", additional_arguments,
      "Additional arguments to pass to the application (format: path=value)");

  try {
    cli_app.parse(argc, argv);
  } catch (const CLI::ParseError &parse_error) {
    cli_app.exit(parse_error);
    return std::nullopt;
  }

  sandbox::properties engine_properties;

  if (!clean_mode) {
    additional_modules.emplace_back("sandbox-application@1.0.0");
  }

  engine_properties.set("booting-configuration/mount-path", mount_path);
  if (logs_name)
    engine_properties.set("booting-configuration/logs-name", *logs_name);
  if (logs_level)
    engine_properties.set("booting-configuration/logs-level", *logs_level);
  if (logs_flush_level)
    engine_properties.set("booting-configuration/logs-flush_level",
                          *logs_flush_level);
  if (logs_pattern)
    engine_properties.set("booting-configuration/logs-pattern", *logs_pattern);
  if (logs_console)
    engine_properties.set("booting-configuration/logs-console_enabled",
                          *logs_console);
  if (logs_filepath)
    engine_properties.set("booting-configuration/logs-filepath",
                          *logs_filepath);
  if (logs_file_truncate)
    engine_properties.set("booting-configuration/logs-file_truncate",
                          *logs_file_truncate);
  if (logs_file_rotating)
    engine_properties.set("booting-configuration/logs-file_rotating",
                          *logs_file_rotating);
  if (logs_file_max_size)
    engine_properties.set("booting-configuration/logs-file_max_size",
                          *logs_file_max_size);
  if (logs_file_max_files)
    engine_properties.set("booting-configuration/logs-file_max_files",
                          *logs_file_max_files);
  if (logs_async)
    engine_properties.set("booting-configuration/logs-async_enabled",
                          *logs_async);
  if (logs_async_queue_size)
    engine_properties.set("booting-configuration/logs-async_queue_size",
                          *logs_async_queue_size);
  if (logs_async_thread_count)
    engine_properties.set("booting-configuration/logs-async_thread_count",
                          *logs_async_thread_count);

  engine_properties.set_array("booting-configuration/additional-libraries",
                              additional_libraries);
  engine_properties.set_array("booting-configuration/additional-modules",
                              additional_modules);

  for (const auto &arg : additional_arguments) {
    size_t pos = arg.find('=');
    if (pos != std::string::npos) {
      std::string key = arg.substr(0, pos);
      std::string value = arg.substr(pos + 1);
      engine_properties.set(key, value);
    }
  }

  return engine_properties;
}

} // namespace sandbox::launcher
