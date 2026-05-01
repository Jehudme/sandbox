#include "engine.h"

#include "plugin.h"
#include "type_registry.h"

namespace sandbox
{
    engine::engine(const properties& manifest) {
        initialize(manifest);
    }

    engine::~engine() {
        finalize();
    }

    void engine::initialize(const properties& manifest)
    {
        ecs.reset();
        ecs.entity("::manifest").set(manifest);

        for (const auto& plugin_type : manifest.list_keys({"plugins"})) {
            std::unique_ptr<plugin> ext = type_registry::instantiate<plugin>(plugin_type, ecs);
            if (ext) ecs.entity(plugin_type.c_str()).emplace<std::unique_ptr<plugin>>(std::move(ext));
            else throw std::runtime_error("Failed to load plugin: " + plugin_type);
        }
    }

    void engine::finalize() {
        ecs.reset();
    }
}
