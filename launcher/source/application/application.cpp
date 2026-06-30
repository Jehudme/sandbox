
#include "application.h"

#include "../../../modules/core/include/sandbox/sdk/configuration.hpp"
#include "../../../modules/core/include/sandbox/sdk/filesystem.hpp"
#include "../../../modules/core/source/configuration/configuration.h"
#include "../../../sandbox/source/core/properties.h"

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
    }

    void ingest_configuration(flecs::world &ecs) {
        std::string content = sandbox::modules::filesystem::read_all_text(ecs, "sandbox://configuration.json");
        sandbox::properties other(content, sandbox::properties::Format::JSON);

        sandbox::modules::configuration::get_properties(ecs).merge("", other);
    }

    application_t::~application_t() = default;
}
