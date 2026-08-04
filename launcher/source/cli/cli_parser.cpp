#include "cli_parser.h"
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace sandbox::launcher {

std::optional<sandbox::properties> parse_cli(int argc, char **argv) {
  CLI::App cli_app{"Sandbox Engine Launcher"};

  std::string app_path;
  cli_app
      .add_option("app_path", app_path,
                  "Path to the application to run (folder or zip)")
      ->required();

  bool dev_mode = false;
  cli_app.add_flag("--dev", dev_mode, "Enable developer mode");

  std::string log_level;
  cli_app.add_option("--logs", log_level,
                     "Set log level (e.g., trace, debug, info, warn, error)");

  try {
    cli_app.parse(argc, argv);
  } catch (const CLI::ParseError &parse_error) {
    cli_app.exit(parse_error);
    return std::nullopt;
  }

  sandbox::properties engine_properties;

  engine_properties.set("filesystem/mounts/app/physical", app_path);
  engine_properties.set("filesystem/mounts/app/readonly", !dev_mode);

  if (dev_mode) {
    engine_properties.set("engine/dev", true);
  }

  if (!log_level.empty()) {
    engine_properties.set("logs/level", log_level);
  }

  // We activate sandbox-configuration, sandbox-logs, sandbox-filesystem,
  // sandbox-runtime, sandbox-application
  std::vector<std::string> modules = {
      "sandbox-configuration@1.0.0", "sandbox-logs@1.0.0",
      "sandbox-filesystem@1.0.0", "sandbox-application@1.0.0"};
  engine_properties.set_array("engine/sandbox", modules);

  return engine_properties;
}

} // namespace sandbox::launcher
