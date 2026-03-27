#pragma once

#include <string_view>
#include <flecs.h>

#include "sandbox/diagnostics/logger.h"

namespace sandbox
{
    class properties;
    class extension;
    class logger;

    class engine
    {
    public:
        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        void initialize(const properties& configuration);

        void create_extension(std::string_view category, std::string_view identifier);
        void delete_extension(std::string_view category);

        template<typename derived_type>
        derived_type* get_extension(std::string_view category);

        void progress();

        template<typename... argument_types>
        void _log(logger::level level, std::string_view format_string, argument_types&&... arguments);

        flecs::world world;
    };
}

#include "engine.inl"