#include "cli_parser.h"
#include "CLI/CLI.hpp"

#include <fstream>
#include "sandbox/core/properties.h"

sandbox_properties_t* sandbox::launcher::parse_cli(int argc, char **argv) {
    CLI::App app{"Sandbox Engine Launcher"};

    std::vector<std::string> libraries;
    app.add_option("-l,--library", libraries, "Libraries to index");

    std::vector<std::string> modules;
    app.add_option("-m,--module", modules, "Modules to activate (format: arch-name@major.minor.patch)");

    std::string config_path;
    app.add_option("-c,--config", config_path, "Configuration file path (JSON)");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        app.exit(e);
        return nullptr;
    }

    auto* props_ptr = sandbox_properties_create();

    if (!config_path.empty()) {
        std::ifstream file(config_path);
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            sandbox_properties_load(props_ptr, content.c_str(), content.size(), SANDBOX_FORMAT_JSON);
        } else {
            std::cerr << "Failed to open configuration file: " << config_path << "\n";
        }
    }

    if (!libraries.empty()) {
        std::vector<const char*> c_libs;
        c_libs.reserve(libraries.size());
        for (const auto& lib : libraries) c_libs.push_back(lib.c_str());
        sandbox_properties_set_string_array(props_ptr, "engine/libraries", c_libs.data(), c_libs.size());
    }

    if (!modules.empty()) {
        std::vector<const char*> c_mods;
        c_mods.reserve(modules.size());
        for (const auto& mod : modules) c_mods.push_back(mod.c_str());
        sandbox_properties_set_string_array(props_ptr, "engine/modules", c_mods.data(), c_mods.size());
    }

    return props_ptr;
}
