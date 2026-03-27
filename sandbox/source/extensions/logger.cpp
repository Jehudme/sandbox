#include "sandbox/extensions/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void logger::initialize(const sandbox::properties& props)
    {
        const std::string name  = props.get<std::string>({"name"}) .value_or("unknown-engine");
        const std::string level = props.get<std::string>({"level"}).value_or("INFO");
        const bool        async = props.get<bool>({"async"})       .value_or(false);

        auto log = std::make_unique<sandbox::logger>(name, sandbox::logger::string_to_level(level), async);

        // Cache the entity and raw pointer before moving the unique_ptr into the ECS store.
        _logger_entity     = _app->world.lookup("::extensions::logger");
        _cached_logger_ptr = log.get();

        _logger_entity.set(std::move(log));
        enable();

        _cached_logger_ptr->info("extensions::logger: initialized name='{}' level='{}'", name, level);
    }

    void logger::finalize()
    {
        // Invalidate the cache so no log calls attempt to use a dangling pointer.
        _cached_logger_ptr = nullptr;
        _logger_entity     = {};
    }

    void logger::enable() const
    {
        if (_logger_entity.is_valid())
        {
            _logger_entity.enable();
        }
    }

    void logger::disable() const
    {
        if (_logger_entity.is_valid())
        {
            _logger_entity.disable();
        }
    }

    bool logger::enabled() const
    {
        return _logger_entity.is_valid() && _logger_entity.enabled();
    }
}

