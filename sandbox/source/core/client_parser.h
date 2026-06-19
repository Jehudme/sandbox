#pragma once

#include "properties.h"
#include "bootstrapper.h"
#include <filesystem>

namespace sandbox::core {

    class client_parser_t {
    public:
        // Returns the properties after reading and parsing the configuration file
        static properties_t load_configuration(const std::filesystem::path& config_path);
        
        // Interprets "engine.libraries" and "engine.modules" to setup the bootstrapper
        static void apply_configuration(bootstrapper_t& bootstrapper, const properties_t& config);
    };

}
