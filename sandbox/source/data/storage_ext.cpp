#include "sandbox/data/storage_ext.h"
#include "sandbox/data/storage_evt.h"
#include "sandbox/ecs/events_ext.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

namespace sandbox::extensions
{
    void storage::initialize(const sandbox::properties& props)
    {
        if (auto* ext_events = _app->get_extension<extensions::events>("events"))
        {
            ext_events->subscribe<sandbox::data::create_object_evt<>>("storage_create_object", [this](sandbox::data::create_object_evt<>& e) {
                if (e.action)
                {
                    e.action(*this, e.name);
                }
            });

            ext_events->subscribe<sandbox::data::get_object_evt<>>("storage_get_object", [this](sandbox::data::get_object_evt<>& e) {
                if (e.action)
                {
                    e.action(*this, e.name, e.out_object);
                }
            });

            ext_events->subscribe<sandbox::data::destroy_object_evt>("storage_destroy_object", [this](sandbox::data::destroy_object_evt& e) {
                this->destroy(e.name);
            });

            ext_events->subscribe<sandbox::data::object_exists_evt>("storage_object_exists", [this](sandbox::data::object_exists_evt& e) {
                if (e.out_exists)
                    *e.out_exists = this->exists(e.name);
            });
        }
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
