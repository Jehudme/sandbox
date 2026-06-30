
#include "application.h"

#include "sandbox/sdk/bootstrapper.hpp"
#include "sandbox/sdk/configuration.hpp"
#include "sandbox/sdk/filesystem.hpp"

#
namespace sandbox::launcher {

    void stage_filesystem(flecs::world& ecs);
    void orchestrate_modules(flecs::world& ecs);
    void ingest_configuration(flecs::world& ecs);

    application_t::application_t(flecs::world &ecs) {
        stage_filesystem(ecs);
        ingest_configuration(ecs);
        orchestrate_modules(ecs);
    }

    void stage_filesystem(flecs::world &ecs) {

    }

    void orchestrate_modules(flecs::world &ecs) {
        std::vector<std::string> plugins_paths = sandbox::modules::filesystem::list_files(ecs, "sandbox://plugins");
        //sandbox::modules::filesystem::re
        for (std::string& plugin_path : plugins_paths) {


        }

    }

    void ingest_configuration(flecs::world &ecs) {
        std::string content = sandbox::modules::filesystem::read_all_text(ecs, "sandbox://configuration.json");
        sandbox::properties other(content, sandbox::properties::Format::JSON);

        sandbox::modules::configuration::get_properties(ecs).merge("", other);
    }

    application_t::~application_t() = default;
}
