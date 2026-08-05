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
      .add_option("mount_path", mount_path,
                  "Path to the application to run (folder or zip)")
      ->required();

  bool dev_mode = false;
  cli_app.add_flag("--dev", dev_mode, "Enable developer mode");

  std::string log_level;
  cli_app.add_option(
      "--logs", log_level,
      "Set log level (e.g., trace, debug, info, warn, error, critical)");

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

  engine_properties.set("filesystem/mounts/app/physical", mount_path);
  engine_properties.set("filesystem/mounts/app/readonly", !dev_mode);
  engine_properties.set("logs/level", log_level.empty() ? "info" : log_level);

  engine_properties.set_array("booting-configuration/libraries",
                              additional_libraries);
  engine_properties.set_array("booting-configuration/modules",
                              additional_modules);

  for (const auto& arg : additional_arguments) {
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
