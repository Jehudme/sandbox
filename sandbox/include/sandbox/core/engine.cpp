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

        world.each<std::unique_ptr<extension>>([this](flecs::entity extension_entity, std::unique_ptr<extension>& extension_pointer) {
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
        world = flecs::world();

        const std::string logger_name = configuration.get<std::string>({"logger", "name"}).value_or("engine_core");
        const std::string logger_level_string = configuration.get<std::string>({"logger", "level"}).value_or("INFO");
        const bool is_logger_async = configuration.get<bool>({"logger", "async"}).value_or(false);

        const logger::level logger_level = sandbox::logger::string_to_level(logger_level_string);

        auto logger_entity = world.entity("::internals::engine_logger");

        logger_entity.set<std::unique_ptr<logger>>(
            std::make_unique<logger>(logger_name, logger_level, is_logger_async)
        );

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

        new_extension->_app = this;
        new_extension->initialize(*this, extension_properties);

        std::string absolute_path = "::extensions::" + std::string(category);
        world.entity(absolute_path.c_str()).set<std::unique_ptr<extension>>({std::move(new_extension)});

        _log(logger::level::debug, "engine: extension created at entity='{}'", absolute_path);
    }

    void engine::delete_extension(std::string_view category)
    {
        const std::string absolute_path = "::extensions::" + std::string(category);
        _log(logger::level::info, "engine: deleting extension category='{}' entity='{}'", category, absolute_path);

        auto extension_entity = world.lookup(absolute_path.c_str());

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

    void engine::progress()
    {
        world.progress();
    }
}