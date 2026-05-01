#include "engine.h"

#include "plugin.h"
#include "type_registry.h"

namespace sandbox
{
    engine::engine() = default;
    engine::~engine() = default;

    void engine::initialize(const properties& manifest)
    {
        ecs.entity("::manifest").set(manifest);
    }

    void engine::finalize()
    {
        ecs.entity("::dependencies").destruct();
    }

    void engine::load_plugin(std::string_view type_name)
    {
        std::unique_ptr<plugin> ext = type_registry::instantiate<plugin>(type_name, ecs);
        std::string path = std::format("::dependecies::{}", type_name);

        ecs.entity(path).set(std::move(ext));
    }

    void engine::unload_plugin(std::string_view type_name)
    {
        std::string path = std::format("::dependecies::{}", type_name);
        ecs.entity(path).destruct();
    }
}
