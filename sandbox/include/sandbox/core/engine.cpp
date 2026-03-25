#include "sandbox/core/engine.h"
#include "sandbox/core/extension.h"
#include "sandbox/filesystem/properties.h"
#include "sandbox/reflections/registry.h"
#include "sandbox/diagnostics/logger.h"

namespace sandbox
{
    engine::engine() = default;

    engine::~engine()
    {
        _log(logger::level::info, "engine: shutting down");

        _world.each<std::unique_ptr<extension>>([this](flecs::entity extension_entity, std::unique_ptr<extension>& extension_pointer) {
            if (extension_pointer)
            {
                _log(logger::level::debug, "engine: finalizing extension entity='{}'", extension_entity.path().c_str());
                extension_pointer->finalize(*this);
            }
        });

        _log(logger::level::info, "engine: shutdown complete");
    }

    void engine::initialize(const properties& configuration)
    {
        _world = flecs::world();

        const std::string logger_name = configuration.get<std::string>({"logger", "name"}).value_or("engine_core");
        const std::string logger_level_string = configuration.get<std::string>({"logger", "level"}).value_or("INFO");
        const bool is_logger_async = configuration.get<bool>({"logger", "async"}).value_or(false);

        const logger::level logger_level = sandbox::logger::string_to_level(logger_level_string);

        // Create/store logger first (so _log works afterwards).
        auto logger_entity = _world.entity("::internals::engine_logger");

        logger_entity.set<std::unique_ptr<logger>>(
            std::make_unique<logger>(logger_name, logger_level, is_logger_async)
        );

        // Now you can safely log using _log()
        _log(logger::level::info, "engine: initialized flecs world");
        _log(logger::level::info, "engine: logger name='{}' level='{}' async={}",
             logger_name, logger::level_to_string(logger_level), is_logger_async);
    }

    void engine::create_extension(std::string_view category, std::string_view identifier)
    {
        _log(logger::level::info, "engine: creating extension category='{}' identifier='{}'", category, identifier);

        std::unique_ptr<extension> new_extension = registry::create_type<extension>(identifier);
        if (!new_extension)
        {
            _log(logger::level::error, "engine: failed to create extension identifier='{}' (registry returned null)", identifier);
            return;
        }

        properties extension_properties;
        new_extension->initialize(*this, extension_properties);

        std::string absolute_path = "::extensions::" + std::string(category);
        _world.entity(absolute_path.c_str()).set<std::unique_ptr<extension>>({std::move(new_extension)});

        _log(logger::level::debug, "engine: extension created at entity='{}'", absolute_path);
    }

    void engine::delete_extension(std::string_view category)
    {
        const std::string absolute_path = "::extensions::" + std::string(category);
        _log(logger::level::info, "engine: deleting extension category='{}' entity='{}'", category, absolute_path);

        auto extension_entity = _world.lookup(absolute_path.c_str());

        if (extension_entity.is_valid() && extension_entity.has<std::unique_ptr<extension>>())
        {
            auto& extension_pointer = extension_entity.get_mut<std::unique_ptr<extension>>();
            if (extension_pointer)
            {
                _log(logger::level::debug, "engine: finalizing extension category='{}'", category);
                extension_pointer->finalize(*this);
            }

            extension_entity.destruct();
            _log(logger::level::debug, "engine: extension entity destructed '{}'", absolute_path);
        }
        else
        {
            _log(logger::level::warn, "engine: extension not found or has no extension component: '{}'", absolute_path);
        }
    }

    void engine::create_stage(std::string_view name, const dependencies& stage_dependencies)
    {
        _log(logger::level::info, "engine: creating stage '{}'", name);

        std::string absolute_path = "::stages::" + std::string(name);
        auto stage_entity = _world.entity(absolute_path.c_str()).add(flecs::Phase);

        for (const auto& dependency_name : stage_dependencies)
        {
            std::string dependency_path = "::stages::" + std::string(dependency_name);
            stage_entity.depends_on(_world.entity(dependency_path.c_str()));
            _log(logger::level::debug, "engine: stage '{}' depends_on '{}'", absolute_path, dependency_path);
        }
    }

    void engine::delete_stage(std::string_view name)
    {
        _log(logger::level::info, "engine: deleting stage '{}'", name);
        _world.lookup(("::stages::" + std::string(name)).c_str()).destruct();
    }

    void engine::enable_stage(std::string_view name)
    {
        _log(logger::level::info, "engine: enabling stage '{}'", name);
        _world.lookup(("::stages::" + std::string(name)).c_str()).enable();
    }

    void engine::disable_stage(std::string_view name)
    {
        _log(logger::level::info, "engine: disabling stage '{}'", name);
        _world.lookup(("::stages::" + std::string(name)).c_str()).disable();
    }

    bool engine::is_stage_exists(std::string_view name) const
    {
        return _world.lookup(("::stages::" + std::string(name)).c_str()).is_valid();
    }

    bool engine::is_stage_enabled(std::string_view name) const
    {
        auto stage_entity = _world.lookup(("::stages::" + std::string(name)).c_str());
        return stage_entity.is_valid() && stage_entity.enabled();
    }

    void engine::delete_system(std::string_view name)
    {
        _log(logger::level::info, "engine: deleting system '{}'", name);
        _world.lookup(("::systems::" + std::string(name)).c_str()).destruct();
    }

    void engine::enable_system(std::string_view name)
    {
        _log(logger::level::info, "engine: enabling system '{}'", name);
        _world.lookup(("::systems::" + std::string(name)).c_str()).enable();
    }

    void engine::disable_system(std::string_view name)
    {
        _log(logger::level::info, "engine: disabling system '{}'", name);
        _world.lookup(("::systems::" + std::string(name)).c_str()).disable();
    }

    bool engine::is_system_exists(std::string_view name) const
    {
        return _world.lookup(("::systems::" + std::string(name)).c_str()).is_valid();
    }

    bool engine::is_system_enabled(std::string_view name) const
    {
        auto system_entity = _world.lookup(("::systems::" + std::string(name)).c_str());
        return system_entity.is_valid() && system_entity.enabled();
    }

    void engine::delete_trigger(std::string_view name)
    {
        _log(logger::level::info, "engine: deleting trigger '{}'", name);
        _world.lookup(("::triggers::" + std::string(name)).c_str()).destruct();
    }

    void engine::enable_trigger(std::string_view name)
    {
        _log(logger::level::info, "engine: enabling trigger '{}'", name);
        _world.lookup(("::triggers::" + std::string(name)).c_str()).enable();
    }

    void engine::disable_trigger(std::string_view name)
    {
        _log(logger::level::info, "engine: disabling trigger '{}'", name);
        _world.lookup(("::triggers::" + std::string(name)).c_str()).disable();
    }

    bool engine::is_trigger_exists(std::string_view name) const
    {
        return _world.lookup(("::triggers::" + std::string(name)).c_str()).is_valid();
    }

    bool engine::is_trigger_enabled(std::string_view name) const
    {
        auto trigger_entity = _world.lookup(("::triggers::" + std::string(name)).c_str());
        return trigger_entity.is_valid() && trigger_entity.enabled();
    }

    void engine::trigger(std::string_view name)
    {
        _log(logger::level::debug, "engine: trigger called '{}'", name);
    }

    void engine::delete_object(std::string_view name)
    {
        _log(logger::level::info, "engine: deleting object '{}'", name);
        _world.lookup(("::objects::" + std::string(name)).c_str()).destruct();
    }

    bool engine::is_object_exists(std::string_view name) const
    {
        return _world.lookup(("::objects::" + std::string(name)).c_str()).is_valid();
    }

    void engine::set_scope(const scope_path& path)
    {
        std::string result = "::";
        for (size_t index = 0; index < path.size(); ++index)
        {
            result += path[index];
            if (index < path.size() - 1) result += "::";
        }

        _log(logger::level::debug, "engine: set_scope '{}'", result);
        _world.set_scope(_world.entity(result.c_str()));
    }

    void engine::push_scope(const scope_path& path)
    {
        std::string result = "::";
        for (size_t index = 0; index < path.size(); ++index)
        {
            result += path[index];
            if (index < path.size() - 1) result += "::";
        }

        _log(logger::level::debug, "engine: push_scope '{}'", result);
        _world.set_scope(_world.entity(result.c_str()));
    }

    void engine::progress()
    {
        _world.progress();
    }
}