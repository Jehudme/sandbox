#include "sandbox/core/engine.h"

#include <iostream>
#include <stack>

#include "sandbox/core/plugin.h"
#include "sandbox/core/type_registry.h"
#include "sandbox/core/scope_guard.h"

namespace sandbox
{
    engine::~engine()
    {
        auto plugins_scope = ecs.lookup("::plugins");
        if (!plugins_scope.is_valid()) return;

        plugins_scope.children([this](flecs::entity child) {
            if (const auto& ptr = child.get<std::unique_ptr<plugin>>()) {
                call_plugin_finalize(ptr.get());
            }
        });
    }

    void engine::call_plugin_initialize(plugin* p) { p->initialize(); }
    void engine::call_plugin_finalize(plugin* p)   { p->finalize();   }

    void engine::initialize(const properties& manifest)
    {
        auto target_manifest = sandbox::properties::parse(R"(
        {
            "plugins": {
                "signals": { "type": "default::signals" }
            }
        }
        )");

        target_manifest.merge(manifest);

        std::vector<std::string> aliases = target_manifest.list_keys({"plugins"});

        for (const std::string& alias : aliases) {
            auto type_name = target_manifest.get<std::string>({"plugins", alias, "type"});
            if (type_name) {
                create_plugin(alias, *type_name);
            } else {
                throw std::runtime_error(std::format("Plugin {} has no type", alias));
            }
        }
    }

    void engine::create_plugin(std::string_view alias, std::string_view type_name)
    {
        SANDBOX_SCOPE_GUARD(ecs.entity("::plugins"));

        // Pass the engine as a strict reference wrapper
        if (auto new_plugin = type_registry::instantiate<plugin, engine&>(type_name, *this)) {
            call_plugin_initialize(new_plugin.get());

            ecs.entity(std::string(alias).c_str())
               .emplace<std::unique_ptr<plugin>>(std::move(new_plugin));
        }
    }

    void engine::delete_plugin(std::string_view alias)
    {
        if (auto plugin_entity = ecs.entity("::plugins").lookup(std::string(alias).c_str())) {
            if (const auto& ptr = plugin_entity.get<std::unique_ptr<plugin>>()) {
                call_plugin_finalize(ptr.get());
            }
            plugin_entity.destruct();
        }
    }

    plugin* engine::get_plugin(std::string_view alias)
    {
        if (auto entt = ecs.entity("::plugins").lookup(std::string(alias).c_str())) {
            if (const auto& plugin_ptr = entt.get<std::unique_ptr<plugin>>()) {
                return plugin_ptr.get();
            }
        }

        return nullptr;
    }
}
