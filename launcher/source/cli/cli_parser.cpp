#include "cli_parser.h"
#include <CLI/CLI.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

namespace sandbox::launcher {

    std::optional<sandbox::properties> parse_cli(int argc, char **argv) {
        CLI::App cli_app{"Sandbox Engine Launcher"};

        // --- Command Line Options Definition ---
        std::vector<std::string> library_paths;
        cli_app.add_option("-l,--library", library_paths, "Paths to libraries to index");

        std::vector<std::string> modules_to_activate;
        cli_app.add_option("-m,--module", modules_to_activate, "Modules to activate (format: arch-name@major.minor.patch)");

        std::string config_file_path;
        cli_app.add_option("-c,--config", config_file_path, "Path to the base configuration file (JSON format)");

        // --- Parse Arguments ---
        try {
            cli_app.parse(argc, argv);
        } catch (const CLI::ParseError& parse_error) {
            cli_app.exit(parse_error);
            return std::nullopt;
        }

        sandbox::properties engine_properties;

        // --- Step 1: Load Base Configuration (File) ---
        if (!config_file_path.empty()) {
            std::ifstream config_file(config_file_path);

            if (!config_file.is_open()) {
                throw std::runtime_error("Failed to open configuration file: " + config_file_path);
            }

            std::string config_content(
                (std::istreambuf_iterator<char>(config_file)),
                std::istreambuf_iterator<char>()
            );

            if (!engine_properties.load(config_content, sandbox::properties::Format::JSON)) {
                throw std::runtime_error("Failed to parse JSON properties from config file: " + config_file_path);
            }
        }

        // --- Step 2: Apply Command Line Overrides (Libraries) ---
        if (!library_paths.empty()) {
            engine_properties.set_array("engine/libraries", library_paths);
        }

        // --- Step 3: Apply Command Line Overrides (Modules) ---
        if (!modules_to_activate.empty()) {
            engine_properties.set_array("engine/sandbox", modules_to_activate);
        }

        return engine_properties;
    }

}
