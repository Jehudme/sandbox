#include "sandbox/extensions/storage.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

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

        const std::string absolute_path = "::objects::" + std::string(name);
        _app->get_logger()->info("extensions::storage: destroying object '{}'", absolute_path);

        auto object_entity = _app->world.lookup(absolute_path.c_str());
        if (!object_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::storage: object '{}' not found", absolute_path);
            return;
        }

        object_entity.destruct();
    }

    bool storage::exists(std::string_view name) const
    {


        const std::string absolute_path = "::objects::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }
}