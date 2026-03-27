#include "../../include/sandbox/extensions/serializer.h"

#include "sandbox/extensions/logger.h"
#include "sandbox/extensions/systems.h"
#include "sandbox/extensions/filesystems.h"
#include "sandbox/core/engine.h"
#include "../../include/sandbox/core/properties.h"

#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>
#include <rttr/registration>
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>
#include <cstring>
#include <cstddef>

namespace sandbox::extensions
{
    namespace
    {
        glz::json_t variant_to_json(const rttr::variant& value)
        {
            glz::json_t output;

            if (value.is_type<bool>())
                output = value.get_value<bool>();
            else if (value.is_type<int>())
                output = value.get_value<int>();
            else if (value.is_type<float>())
                output = static_cast<double>(value.get_value<float>());
            else if (value.is_type<double>())
                output = value.get_value<double>();
            else if (value.is_type<std::string>())
                output = value.get_value<std::string>();
            else
                output = value.to_string();

            return output;
        }

        bool set_property_from_json(rttr::instance target, const rttr::property& property, const glz::json_t& json_value)
        {
            const auto type = property.get_type();

            if (type == rttr::type::get<bool>() && json_value.holds<bool>())
                return property.set_value(target, json_value.get<bool>());
            if (type == rttr::type::get<int>() && json_value.holds<double>())
                return property.set_value(target, static_cast<int>(json_value.get<double>()));
            if (type == rttr::type::get<float>() && json_value.holds<double>())
                return property.set_value(target, static_cast<float>(json_value.get<double>()));
            if (type == rttr::type::get<double>() && json_value.holds<double>())
                return property.set_value(target, json_value.get<double>());
            if (type == rttr::type::get<std::string>() && json_value.holds<std::string>())
                return property.set_value(target, json_value.get<std::string>());

            // Fallback to string conversion when possible
            if (json_value.holds<std::string>())
            {
                return property.set_value(target, json_value.get<std::string>());
            }

            return false;
        }

        std::optional<glz::json_t> serialize_entity(flecs::entity entity, sandbox::extensions::logger* log)
        {
            glz::json_t root = glz::json_t::object_t{};
            root["name"] = entity.name() ? entity.name() : "";

            glz::json_t components = glz::json_t::object_t{};

            entity.each([&](flecs::id comp_id)
            {
                if (comp_id.is_pair())
                    return;

                flecs::entity comp_entity(entity.world(), comp_id);
                if (!comp_entity.is_valid())
                    return;

                const char* comp_name = comp_entity.name();
                if (!comp_name || comp_name[0] == '\0')
                    return;

                auto comp_type = rttr::type::get_by_name(comp_name);
                if (!comp_type.is_valid())
                {
                    if (log) log->debug("extensions::serializer: component '{}' is not registered with RTTR", comp_name);
                    return;
                }

                const void* data_ptr = ecs_get_id(entity.world(), entity.id(), comp_entity.id());
                const auto comp_size = comp_type.get_raw_type().get_sizeof();
                if (!data_ptr || comp_size == 0)
                    return;

                // Snapshot component data to avoid mutating live ECS storage while reflecting.
                // WARNING: this generic serializer expects components to be trivially copyable; complex components
                // should supply specialized serialization to avoid undefined behavior.
                if (!(comp_type.is_arithmetic() || comp_type.is_enumeration()))
                {
                    if (log) log->warn("extensions::serializer: '{}' is not trivially copyable; skipping generic serialization", comp_name);
                    return;
                }
                std::vector<std::byte> component_snapshot(comp_size);
                std::memcpy(component_snapshot.data(), data_ptr, comp_size);

                glz::json_t component_json = glz::json_t::object_t{};

                rttr::instance instance(component_snapshot.data());
                for (auto& prop : comp_type.get_properties())
                {
                    auto value = prop.get_value(instance);
                    component_json[prop.get_name().to_string()] = variant_to_json(value);
                }

                components[comp_name] = component_json;
            });

            root["components"] = components;
            return root;
        }

        void ensure_directory(const std::filesystem::path& path)
        {
            std::filesystem::create_directories(path.parent_path());
        }

        serializer::config load_config(flecs::world world)
        {
            auto serializer_entity = world.lookup("::extensions::serializer");
            if (serializer_entity.is_valid() && serializer_entity.has<serializer::config>())
            {
                return serializer_entity.get<serializer::config>();
            }

            if (world.has<serializer::config>())
            {
                return world.get<serializer::config>();
            }

            return serializer::config{};
        }
    }

    void serializer::initialize(const sandbox::properties& properties)
    {
        if (!_app) return;

        auto* log = _app->get_logger();

        _app->world.component<save_request>();
        _app->world.component<load_request>();
        _app->world.component<persistent_tag>();
        _app->world.component<config>();

        config cfg{
            .root_dir = properties.get<std::string>({"root_dir"}).value_or("saves")
        };

        std::filesystem::create_directories(cfg.root_dir);

        if (auto serializer_entity = _app->world.lookup("::extensions::serializer"); serializer_entity.is_valid())
        {
            serializer_entity.set<config>(cfg);
        }
        else
        {
            _app->world.set<config>(cfg);
        }

        auto* ext_systems = _app->get_extension<extensions::systems>("systems");
        if (!ext_systems)
        {
            if (log) log->error("extensions::serializer: systems extension unavailable");
            return;
        }

        ext_systems->create<save_request>(
            "serializer_save",
            "",
            [](auto& builder) { builder.kind(flecs::PostUpdate); },
            [this](flecs::iter& it)
            {
                auto* log = _app ? _app->get_logger() : nullptr;
                auto cfg = load_config(it.world());
                const auto root = std::filesystem::path(cfg.root_dir);
                std::vector<flecs::entity> completed_requests;

                while (it.next())
                {
                    auto requests = it.field<save_request>(0);
                    for (auto row : it)
                    {
                        auto entity = it.entity(row);
                        auto serialized = serialize_entity(entity, log);
                        if (!serialized) continue;

                        std::string output;
                        auto err = glz::write_json(*serialized, output);
                        if (err)
                        {
                            if (log) log->error("extensions::serializer: failed to serialize entity '{}'", entity.name().c_str());
                            continue;
                        }

                        std::filesystem::path out_path = root / requests[row].filename;
                        ensure_directory(out_path);

                        std::ofstream stream(out_path, std::ios::binary);
                        stream << output;

                        if (log) log->info("extensions::serializer: saved entity '{}' -> '{}'", entity.name().c_str(), out_path.string());
                        completed_requests.push_back(entity);
                    }
                }

                for (auto& entity : completed_requests)
                {
                    entity.remove<save_request>();
                }
            }
        );

        ext_systems->create<load_request>(
            "serializer_load",
            "",
            [](auto& builder) { builder.kind(flecs::PostUpdate); },
            [this](flecs::iter& it)
            {
                auto* log = _app ? _app->get_logger() : nullptr;
                auto cfg = load_config(it.world());
                const auto root = std::filesystem::path(cfg.root_dir);
                std::vector<flecs::entity> completed_requests;

                while (it.next())
                {
                    auto requests = it.field<load_request>(0);
                    for (auto row : it)
                    {
                        auto entity = it.entity(row);
                        std::filesystem::path in_path = root / requests[row].filename;

                        std::ifstream stream(in_path, std::ios::binary);
                        if (!stream)
                        {
                            if (log) log->warn("extensions::serializer: could not open '{}'", in_path.string());
                            continue;
                        }

                        std::string json_content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                        glz::json_t json_tree;
                        if (auto err = glz::read_json(json_tree, json_content))
                        {
                            if (log) log->error("extensions::serializer: failed to parse '{}'", in_path.string());
                            continue;
                        }

                        if (!json_tree.contains("components"))
                            continue;

                        for (const auto& [comp_name, comp_data] : json_tree["components"].get_object())
                        {
                            auto comp_type = rttr::type::get_by_name(comp_name.c_str());
                            if (!comp_type.is_valid())
                            {
                                if (log) log->warn("extensions::serializer: component '{}' is not registered", comp_name);
                                continue;
                            }

                            auto comp_entity = it.world().lookup(comp_name.c_str());
                            if (!comp_entity.is_valid())
                            {
                                comp_entity = it.world().component(comp_name.c_str());
                            }

                            void* comp_ptr = ecs_get_mut_id(it.world().c_ptr(), entity.id(), comp_entity.id());
                            if (!comp_ptr)
                                continue;

                            rttr::instance inst(comp_ptr);
                            for (auto& prop : comp_type.get_properties())
                            {
                                auto it_prop = comp_data.get_object().find(prop.get_name().to_string());
                                if (it_prop != comp_data.get_object().end())
                                {
                                    set_property_from_json(inst, prop, it_prop->second);
                                }
                            }
                        }

                        if (log) log->info("extensions::serializer: loaded entity '{}' from '{}'", entity.name().c_str(), in_path.string());
                        completed_requests.push_back(entity);
                    }
                }

                for (auto& entity : completed_requests)
                {
                    entity.remove<load_request>();
                }
            }
        );

        if (log) log->info("extensions::serializer: initialized");
    }

    void serializer::finalize()
    {
        if (!_app) return;

        if (auto* ext_systems = _app->get_extension<extensions::systems>("systems"))
        {
            ext_systems->destroy("serializer_save");
            ext_systems->destroy("serializer_load");
        }

        _app->world.each<save_request>([](flecs::entity e, save_request&) { e.remove<save_request>(); });
        _app->world.each<load_request>([](flecs::entity e, load_request&) { e.remove<load_request>(); });
    }
}
