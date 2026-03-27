#include "sandbox/io/filesystem_ext.h"
#include "sandbox/diagnostics/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

#include <filesystem>

namespace sandbox::extensions
{
    void filesystem::initialize(const sandbox::properties& properties)
    {
        if (!_app) return;

        auto* log = _app->get_logger();

        _app->world.component<state>();
        _app->world.component<file_changed_event>();
        _app->world.component<read_request>();

        state mounts_state{};

        // Load mounts from configuration: properties["mounts"][virtual_prefix] = physical_path
        for (const auto& mount_key : properties.get_keys({"mounts"}))
        {
            auto resolved_path = properties.get<std::string>({"mounts", mount_key});
            if (resolved_path)
            {
                mounts_state.mounts[mount_key] = *resolved_path;
                if (log) log->debug("extensions::filesystem: mount '{}' -> '{}'", mount_key, *resolved_path);
            }
        }

        // Provide a sensible default if none were configured
        if (mounts_state.mounts.empty())
        {
            const auto default_res_root = std::filesystem::current_path() / "res";
            mounts_state.mounts["res://"] = default_res_root.string();
            if (log) log->info("extensions::filesystem: default mount 'res://' -> '{}'", mounts_state.mounts["res://"]);
        }

        if (auto fs_entity = _app->world.lookup("::extensions::filesystem"); fs_entity.is_valid())
        {
            fs_entity.set<state>(mounts_state);
        }

        if (log) log->info("extensions::filesystem: initialized");
    }

    void filesystem::finalize()
    {
        if (!_app) return;
        if (auto* log = _app->get_logger()) log->info("extensions::filesystem: finalized");
    }

    std::string filesystem::resolve(std::string_view path) const
    {
        if (!_app) return std::string(path);

        const auto fs_entity = _app->world.lookup("::extensions::filesystem");
        const state* mounts_state = (fs_entity.is_valid() && fs_entity.has<state>())
            ? &fs_entity.get<state>()
            : nullptr;

        if (!mounts_state)
            return std::string(path);

        const std::string incoming(path);

        for (const auto& [virtual_prefix, physical_root] : mounts_state->mounts)
        {
            if (incoming.compare(0, virtual_prefix.size(), virtual_prefix) == 0)
            {
                std::filesystem::path resolved = physical_root;
                resolved /= incoming.substr(virtual_prefix.size());
                return std::filesystem::weakly_canonical(resolved).string();
            }
        }

        return incoming;
    }
}
