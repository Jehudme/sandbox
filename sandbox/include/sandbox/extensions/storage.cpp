#include "sandbox/extensions/storage.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void storage::initialize(sandbox::engine& app, const sandbox::properties& props)
    {
        app._log(sandbox::logger::level::info, "extensions::storage: initialized");
    }

    void storage::finalize(sandbox::engine& app)
    {
        app._log(sandbox::logger::level::info, "extensions::storage: finalized");
    }

    void storage::destroy(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::objects::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::storage: destroying object '{}'", absolute_path);

        auto object_entity = _app->world.lookup(absolute_path.c_str());
        if (!object_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::storage: object '{}' not found", absolute_path);
            return;
        }

        object_entity.destruct();
    }

    bool storage::exists(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::objects::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }
}