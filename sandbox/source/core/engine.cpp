#include "sandbox/core/engine.h"

#include <stack>

#include "sandbox/core/plugin.h"
#include "sandbox/core/type_registry.h"
#include "sandbox/core/scope_guard.h"

namespace sandbox
{
    void engine::initialize(const properties& manifest)
    {
        SANDBOX_SCOPE_GUARD(ecs.entity("::plugins"));

        std::vector<std::string> aliases = manifest.list_keys({"plugins"});

        for (std::string alias : aliases) {
            std::optional<std::string> type_name = manifest.get<std::string>({"plugins", alias, "type"});

            if (type_name.has_value()) create_plugin(alias, *type_name);
            else throw std::runtime_error(std::format("Plugin {} has no type specified", alias));
        }
    }

    void engine::create_plugin(std::string_view alias, std::string_view type_name)
    {
        SANDBOX_SCOPE_GUARD(ecs.entity("::plugins"));

        if (auto new_plugin = type_registry::instantiate<plugin>(type_name, this)) {
            ecs.entity(alias.data()).emplace<std::unique_ptr<plugin>>(std::move(new_plugin));
        }
    }

    void engine::delete_plugin(std::string_view alias)
    {
        SANDBOX_SCOPE_GUARD(ecs.entity("::plugins"));

        if (auto plugin_entity = ecs.lookup(alias.data())){
            plugin_entity.destruct();
        }
    }

    plugin* engine::get_plugin(std::string_view alias)
    {
        SANDBOX_SCOPE_GUARD(ecs.entity("::plugins"));

        if (auto entt = ecs.lookup(alias.data())) {
            if (auto& plugin_ptr = entt.get<std::unique_ptr<plugin>>()) {
                return plugin_ptr.get();
            }
        }

        return nullptr;
    }
}
