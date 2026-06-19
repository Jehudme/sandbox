#include "client_parser.h"
#include <fstream>
#include <sstream>

namespace sandbox::core {

    properties_t client_parser_t::load_configuration(const std::filesystem::path& config_path) {
        properties_t props;
        if (!std::filesystem::exists(config_path)) {
            return props;
        }
        
        std::ifstream file(config_path);
        if (!file.is_open()) {
            return props;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        
        properties_t::Format fmt = properties_t::JSON;
        std::string ext = config_path.extension().string();
        if (ext == ".json") fmt = properties_t::JSON;
        else if (ext == ".toml") fmt = properties_t::TOML;
        else if (ext == ".yaml" || ext == ".yml") fmt = properties_t::YAML;
        else if (ext == ".beve") fmt = properties_t::BEVE;
        
        props.load(buffer.str(), fmt);
        return props;
    }

    void client_parser_t::apply_configuration(bootstrapper_t& bootstrapper, const properties_t& config) {
        // Load libraries
        if (auto libraries = config.get<std::vector<std::string>>({"engine", "libraries"})) {
            for (const auto& lib : *libraries) {
                bootstrapper_t::index_library(lib);
            }
        }

        // Activate modules
        if (auto modules = config.get<std::vector<std::string>>({"engine", "modules"})) {
            for (const auto& mod : *modules) {
                bootstrapper.activate(mod);
            }
        }
    }

}
