#include "sandbox/extensions/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void logger::initialize(const sandbox::properties& props)
    {
        const std::string   name    = props.get<std::string>({"name"})    .value_or("unknow-engine");
        const std::string   level   = props.get<std::string>({"level"})   .value_or("INFO");
        const bool          async   = props.get<bool>({"async"})          .value_or(false);

        auto logger = std::make_unique<sandbox::logger>(name, sandbox::logger::string_to_level(level), async);

        _app->world.lookup("::extensions::logger").set(std::move(logger)); enable();
        _app->get_logger()->info("extensions::logger: initialized name='{}' level='{}'", name, level);
    }

    void logger::finalize()
    {
    }

    void logger::enable() const
    {
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (extension_logger_entity.is_valid())
        {
            extension_logger_entity.enable();
        }
    }

    void logger::disable() const
    {
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        if (extension_logger_entity.is_valid())
        {
            extension_logger_entity.disable();
        }
    }

    bool logger::enabled() const
    {
        flecs::entity extension_logger_entity = _app->world.lookup("::extensions::logger");
        return extension_logger_entity.is_valid() && extension_logger_entity.enabled();
    }
}
