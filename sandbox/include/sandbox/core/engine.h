#pragma once

#include <string_view>
#include <flecs.h>

#include "properties.h"

namespace sandbox
{
    namespace extensions
    {
        class triggers;
        class scopes;
        class storage;
        class systems;
        class events;
        class caches;
        class clock;
        class filesystem;
        class dependencies;
        class serializer;
        class diagnostics;
    }

    class properties;
    class extension;
    class logger;

    class engine
    {
    public:
        engine();
        virtual ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        virtual void initialize(const properties& configuration);
        virtual void finalize();

        void create_extension(std::string_view category, std::string_view identifier);
        void delete_extension(std::string_view category);

        void initialize_extension(std::string_view name, const properties& props = properties());
        void finalize_extension(std::string_view name);

        template<typename derived_type>
        derived_type* get_extension(std::string_view category);

        sandbox::logger* get_logger();
        extensions::scopes* get_scopes();
        extensions::storage* get_storage();
        extensions::systems* get_systems();
        extensions::events* get_events();
        extensions::triggers* get_triggers();
        extensions::caches* get_caches();
        extensions::clock* get_clock();
        extensions::filesystem* get_filesystem();
        extensions::dependencies* get_dependencies();
        extensions::serializer* get_serializer();
        extensions::diagnostics* get_diagnostics();

        void progress();

        flecs::world world;
    };
}

#include "engine.inl"
