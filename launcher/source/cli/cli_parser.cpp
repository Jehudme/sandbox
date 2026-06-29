#include "cli_parser.h"
#include <CLI/CLI.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "../../../sandbox/include/sandbox/abi/properties.h"

sandbox_properties_handle_t sandbox::launcher::parse_cli(int argc, char **argv) {
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
        return sandbox_properties_handle_t{0};
    }

    sandbox_properties_handle_t engine_properties = sandbox_properties_create();

    // --- Step 1: Load Base Configuration (File) ---
    if (!config_file_path.empty()) {
        std::ifstream config_file(config_file_path);

        if (config_file.is_open()) {
            std::string config_content(
                (std::istreambuf_iterator<char>(config_file)),
                std::istreambuf_iterator<char>()
            );

            if (!sandbox_properties_load(
                engine_properties,
                config_content.c_str(),
                config_content.size(),
                SANDBOX_FORMAT_JSON
            )) {
                std::cerr << "[Launcher] Failed to load json properties. Config content:\n" << config_content << "\n";
            }
        } else {
            std::cerr << "[Launcher] Failed to open configuration file: " << config_file_path << "\n";
        }
    }

    // --- Step 2: Apply Command Line Overrides (Libraries) ---
    if (!library_paths.empty()) {
        std::vector<const char*> c_style_library_paths;
        c_style_library_paths.reserve(library_paths.size());

        for (const std::string& path : library_paths) {
            c_style_library_paths.push_back(path.c_str());
        }

        sandbox_properties_set_string_array(
            engine_properties,
            "engine/libraries",
            c_style_library_paths.data(),
            c_style_library_paths.size()
        );
    }

    // --- Step 3: Apply Command Line Overrides (Modules) ---
    if (!modules_to_activate.empty()) {
        std::vector<const char*> c_style_modules;
        c_style_modules.reserve(modules_to_activate.size());

        for (const std::string& module_target : modules_to_activate) {
            c_style_modules.push_back(module_target.c_str());
        }

        sandbox_properties_set_string_array(
            engine_properties,
            "engine/sandbox",
            c_style_modules.data(),
            c_style_modules.size()
        );
    }

    return engine_properties;
}
