#include "sandbox/core/engine.h"
#include "sandbox/core/extension.h"
#include "sandbox/filesystem/properties.h"
#include "sandbox/reflections/registry.h"
#include "sandbox/reflections/registration.h"
#include "sandbox/extensions/logger.h"
#include "sandbox/extensions/scopes.h"
#include "sandbox/extensions/stages.h"
#include "sandbox/extensions/systems.h"
#include "sandbox/extensions/triggers.h"
#include "sandbox/extensions/storage.h"
#include "sandbox/extensions/events.h"

namespace sandbox
{
    engine::engine() = default;

    engine::~engine()
    {
        finalize();
    }

    void engine::initialize(const properties& configuration)
    {
        world = flecs::world();

        create_extension("logger", "default_logger_extension");
        create_extension("scopes", "default_scopes_extension");
        create_extension("stages", "default_stages_extension");
        create_extension("systems", "default_systems_extension");
        create_extension("triggers", "default_triggers_extension");
        create_extension("storage", "default_storage_extension");
        create_extension("events", "default_events_extension");

        initialize_extension("logger", configuration.sub_properties({"logger"}));
        initialize_extension("scopes");
        initialize_extension("stages");
        initialize_extension("systems");
        initialize_extension("triggers");
        initialize_extension("storage");
        initialize_extension("events");
        
        get_extension<extensions::logger>("logger")->info("engine: initialized");
    }

    void engine::finalize()
    {
        if (auto* ext_logger = get_logger())
        {
            ext_logger->info("engine: finalizing");
        }

        delete_extension("events");
        delete_extension("storage");
        delete_extension("triggers");
        delete_extension("systems");
        delete_extension("stages");
        delete_extension("scopes");
        delete_extension("logger");
    }

    void engine::create_extension(std::string_view category, std::string_view identifier)
    {
        if (get_logger()) get_logger()->info("engine: creating extension category='{}' identifier='{}'", category, identifier);

        std::unique_ptr<extension> new_extension = registry::create_type<extension>(identifier);
        if (!new_extension)
        {
            if(get_logger()) get_logger()->error("engine: failed to create extension identifier='{}' (registry returned null)", identifier);
            return;
        }

        std::string absolute_path = "::extensions::" + std::string(category);
        world.entity(absolute_path.c_str()).set<std::unique_ptr<extension>>({std::move(new_extension)});

        if (get_logger()) get_logger()->debug("engine: extension created at entity='{}'", absolute_path);
    }

    void engine::delete_extension(std::string_view category)
    {
        const std::string absolute_path = "::extensions::" + std::string(category);
        if(get_logger()) get_logger()->info( "engine: deleting extension category='{}' entity='{}'", category, absolute_path);

        auto extension_entity = world.lookup(absolute_path.c_str());

        if (extension_entity.is_valid() && extension_entity.has<std::unique_ptr<extension>>())
        {
            auto& extension_pointer = extension_entity.get_mut<std::unique_ptr<extension>>();
            if (extension_pointer)
            {
                if(get_logger()) get_logger()->debug("engine: finalizing extension category='{}'", category);
                extension_pointer->finalize();
            }

            extension_entity.destruct();
            if(get_logger()) get_logger()->debug("engine: extension entity destructed '{}'", absolute_path);
        }
        else
        {
            if(get_logger()) get_logger()->warn("engine: extension not found or has no extension component: '{}'", absolute_path);
        }
    }

    void engine::initialize_extension(std::string_view name, const properties& configuration)
    {
        std::string absolute_path = "::extensions::" + std::string(name);
        if (auto entity = world.lookup(absolute_path.c_str()); entity.is_valid() && entity.has<std::unique_ptr<extension>>())
        {
            entity.get_mut<std::unique_ptr<extension>>()->_app = this;
            entity.get_mut<std::unique_ptr<extension>>()->initialize(configuration);
            return;
        }

        if (get_logger()) get_logger()->error("engine: extension not found or has no extension component: '{}'", absolute_path);
    }

    void engine::finalize_extension(std::string_view name)
    {
        if (extension* ext = get_extension<extension>(name))
        {
            ext->finalize();
            ext->_app = nullptr;
             if (get_logger()) get_logger()->debug("engine: finalized extension '{}'", name);
             return;
        }
        if (get_logger()) get_logger()->error("engine: extension not found for finalize: '{}'", name);
    }

    extensions::logger* engine::get_logger()        { return get_extension<extensions::logger>("logger"); }
    extensions::scopes* engine::get_scopes()        { return get_extension<extensions::scopes>("scopes"); }
    extensions::storage* engine::get_storage()      { return get_extension<extensions::storage>("storage"); }
    extensions::systems* engine::get_systems()      { return get_extension<extensions::systems>("systems"); }
    extensions::events* engine::get_events()        { return get_extension<extensions::events>("events"); }
    extensions::triggers* engine::get_triggers()    { return get_extension<extensions::triggers>("triggers"); }

    void engine::progress()                         { world.progress(); }
}



SANDBOX_REGISTRATION {
    SANDBOX_REGISTER_CLASS(sandbox::extensions::logger, "default_logger_extension")
    SANDBOX_REGISTER_CLASS(sandbox::extensions::scopes, "default_scopes_extension")
    SANDBOX_REGISTER_CLASS(sandbox::extensions::stages, "default_stages_extension")
    SANDBOX_REGISTER_CLASS(sandbox::extensions::systems, "default_systems_extension")
    SANDBOX_REGISTER_CLASS(sandbox::extensions::triggers, "default_triggers_extension")
    SANDBOX_REGISTER_CLASS(sandbox::extensions::storage, "default_storage_extension")
    SANDBOX_REGISTER_CLASS(sandbox::extensions::events, "default_events_extension")
}