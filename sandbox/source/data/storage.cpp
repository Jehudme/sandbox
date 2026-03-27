#include "sandbox/data/storage.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

namespace sandbox::extensions
{
    void storage::initialize(const sandbox::properties& props)
    {
    }

    void storage::finalize()
    {
    }

    void storage::destroy(std::string_view name)
    {
        if (!_app) return;

        const std::string absolute_path = "::objects::" + std::string(name);

        if (auto* log = _app->get_logger())
            log->info("extensions::storage: destroying object '{}'", absolute_path);

        auto object_entity = _app->world.lookup(absolute_path.c_str());
        if (!object_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::storage: object '{}' not found", absolute_path);
            return;
        }

        object_entity.destruct();
    }

    bool storage::exists(std::string_view name) const
    {
        if (!_app) return false;

        const std::string absolute_path = "::objects::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }
}