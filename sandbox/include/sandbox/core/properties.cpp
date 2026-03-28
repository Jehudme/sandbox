#include "properties.h"
#include <glaze/glaze.hpp>
#include <fstream>
#include <sstream>

namespace sandbox
{
    void properties::load_from_file(const std::filesystem::path& file_path)
    {
        std::ifstream file_stream(file_path);
        if (!file_stream.is_open()) return;

        std::stringstream string_buffer;
        string_buffer << file_stream.rdbuf();
        glz::read_json(m_root_node, string_buffer.str());
    }

    void properties::save_to_file(const std::filesystem::path& file_path) const
    {
        std::ofstream file_stream(file_path);
        if (!file_stream.is_open()) return;

        std::string json_output_buffer;
        glz::write_json(m_root_node, json_output_buffer);
        file_stream << json_output_buffer;
    }

    std::string properties::to_json_string() const
    {
        std::string json_output_string;
        glz::write_json(m_root_node, json_output_string);
        return json_output_string;
    }

    void properties::merge(const properties& other_properties)
    {
        deep_merge(m_root_node, other_properties.m_root_node);
    }

    void properties::deep_merge(glz::json_t& destination, const glz::json_t& source)
    {
        if (source.is_object() && destination.is_object()) {
            auto& dest_map = destination.get_object();
            const auto& src_map = source.get_object();

            for (const auto& [key, value] : src_map) {
                deep_merge(dest_map[key], value);
            }
        } else {
            // If they aren't both objects, the source value overwrites the destination
            destination = source;
        }
    }

    void properties::move(const key_path& source_path, const key_path& destination_path)
    {
        if (source_path == destination_path || source_path.empty()) return;

        glz::json_t* source_parent_node_ptr = &m_root_node;
        for (size_t i = 0; i < source_path.size() - 1; ++i) {
            if (!source_parent_node_ptr->is_object()) return;
            source_parent_node_ptr = &source_parent_node_ptr->get_object()[source_path[i]];
        }

        auto& source_object_map = source_parent_node_ptr->get_object();
        auto source_iterator = source_object_map.find(source_path.back());

        if (source_iterator == source_object_map.end()) return;

        glz::json_t extracted_json_node = std::move(source_iterator->second);
        source_object_map.erase(source_iterator);

        glz::json_t* destination_node_ptr = &m_root_node;
        for (const std::string& key : destination_path) {
            if (!destination_node_ptr->is_object()) {
                *destination_node_ptr = glz::json_t::object_t{};
            }
            destination_node_ptr = &destination_node_ptr->get_object()[key];
        }
        *destination_node_ptr = std::move(extracted_json_node);
    }

    void properties::rename(const key_path& path, const std::string& new_name)
    {
        if (path.empty() || new_name.empty()) return;
        key_path renamed_key_path = path;
        renamed_key_path.back() = new_name;
        move(path, renamed_key_path);
    }

    void properties::remove(const key_path& path)
    {
        if (path.empty()) return;
        glz::json_t* current_node_ptr = &m_root_node;
        for (size_t i = 0; i < path.size() - 1; ++i) {
            if (!current_node_ptr->is_object()) return;
            current_node_ptr = &current_node_ptr->get_object()[path[i]];
        }
        if (current_node_ptr->is_object()) {
            current_node_ptr->get_object().erase(path.back());
        }
    }

    void properties::clear() noexcept
    {
        m_root_node = glz::json_t::object_t{};
    }

    bool properties::contains(const key_path& path) const
    {
        const glz::json_t* current_node_ptr = &m_root_node;
        for (const std::string& key : path) {
            if (current_node_ptr->is_object() && current_node_ptr->get_object().contains(key)) {
                current_node_ptr = &current_node_ptr->get_object().at(key);
            } else return false;
        }
        return true;
    }

    properties properties::get_subtree(const key_path& path) const
    {
        properties subtree_result;
        const glz::json_t* current_node_ptr = &m_root_node;
        for (const std::string& key : path) {
            if (current_node_ptr->is_object() && current_node_ptr->get_object().contains(key)) {
                current_node_ptr = &current_node_ptr->get_object().at(key);
            } else return subtree_result;
        }
        subtree_result.m_root_node = *current_node_ptr;
        return subtree_result;
    }

    properties::key_list properties::list_keys(const key_path& path) const
    {
        key_list key_list_result;
        const glz::json_t* current_node_ptr = &m_root_node;
        for (const std::string& key : path) {
            if (current_node_ptr->is_object() && current_node_ptr->get_object().contains(key)) {
                current_node_ptr = &current_node_ptr->get_object().at(key);
            } else return {};
        }

        if (current_node_ptr->is_object()) {
            const auto& object_map = current_node_ptr->get_object();
            key_list_result.reserve(object_map.size());
            for (const auto& [key, value] : object_map) {
                key_list_result.push_back(key);
            }
        }
        return key_list_result;
    }

    void properties::traverse(const visitor_callback& callback) const
    {
        key_path active_traversal_path;
        walk(m_root_node, active_traversal_path, callback);
    }

    void properties::walk(const glz::json_t& current_node, key_path& current_path, const visitor_callback& callback) const
    {
        std::string serialized_json_value;
        glz::write_json(current_node, serialized_json_value);
        callback(current_path, serialized_json_value);

        if (current_node.is_object()) {
            for (const auto& [key, child_node] : current_node.get_object()) {
                current_path.push_back(key);
                walk(child_node, current_path, callback);
                current_path.pop_back();
            }
        }
    }
}