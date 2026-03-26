#pragma once

#include <string_view>
#include <vector>
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
        using dependencies = std::vector<std::string_view>;
        using scope_path = std::vector<std::string_view>;

        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        void initialize(const properties& configuration);

        void create_extension(std::string_view category, std::string_view identifier);
        void delete_extension(std::string_view category);

        template<typename derived_type>
        derived_type* get_extension(std::string_view category);

        void create_stage(std::string_view name, const dependencies& stage_dependencies = {});
        void delete_stage(std::string_view name);
        void enable_stage(std::string_view name);
        void disable_stage(std::string_view name);

        bool is_stage_exists(std::string_view name) const;
        bool is_stage_enabled(std::string_view name) const;

        template<typename... components>
        void create_system(std::string_view name, std::string_view stage, auto&& configuration_lambda, auto&& logic_lambda);

        template<typename... components>
        void create_system(std::string_view name, std::string_view stage, auto&& logic_lambda);

        void delete_system(std::string_view name);
        void enable_system(std::string_view name);
        void disable_system(std::string_view name);

        bool is_system_exists(std::string_view name) const;
        bool is_system_enabled(std::string_view name) const;

        template<typename... components>
        void create_trigger(std::string_view name, auto&& configuration_lambda, auto&& logic_lambda);

        void delete_trigger(std::string_view name);
        void enable_trigger(std::string_view name);
        void disable_trigger(std::string_view name);

        bool is_trigger_exists(std::string_view name) const;
        bool is_trigger_enabled(std::string_view name) const;

        void trigger(std::string_view name);

        void set_scope(const scope_path& desired_path);
        void push_scope(const scope_path& desired_path);

        template<typename base_type, typename... component_args>
        void create_object(std::string_view name, component_args&&... initialization_arguments);

        void delete_object(std::string_view name);

        template<typename base_type>
        base_type* get_object(std::string_view name);

        bool is_object_exists(std::string_view name) const;

        void progress();

        template<typename... argument_types>
        void _log(logger::level level, std::string_view format_string, argument_types&&... arguments);

        flecs::world world;
    };
}

#include "engine.inl"