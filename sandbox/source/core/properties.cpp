#include "sandbox/utilities/properties.h"
#include <glaze/glaze.hpp>

namespace sandbox
{
    properties::properties(std::string_view json_string)
    {
        (void)load_from_string(json_string);
    }

    properties::properties(const std::vector<std::byte>& byte_data)
    {
        (void)load_from_bytes(byte_data);
    }

    std::expected<void, std::string> properties::load_from_bytes(const std::vector<std::byte>& byte_data)
    {
        if (byte_data.empty()) {
            m_root_node = glz::json_t::object_t{};
            return {};
        }

        // Cast the raw byte buffer into a string_view for instant parsing without copies
        std::string_view json_view(reinterpret_cast<const char*>(byte_data.data()), byte_data.size());
        return load_from_string(json_view);
    }

    std::expected<void, std::string> properties::load_from_string(std::string_view json_string)
    {
        m_root_node = glz::json_t::object_t{}; // Reset the tree
        const auto error_context = glz::read_json(m_root_node, json_string);
        if (error_context) {
            m_root_node = glz::json_t::object_t{};
            return std::unexpected(glz::format_error(error_context, json_string));
        }
        return {};
    }

    std::expected<properties, std::string> properties::parse(std::string_view json_string)
    {
        properties p;
        auto res = p.load_from_string(json_string);
        if (!res) return std::unexpected(res.error());
        return p;
    }

    std::expected<properties, std::string> properties::parse(const std::vector<std::byte>& byte_data)
    {
        properties p;
        auto res = p.load_from_bytes(byte_data);
        if (!res) return std::unexpected(res.error());
        return p;
    }

    std::string properties::save_to_string(const key_path& path) const
    {
        const glz::json_t* current_node_ptr = &m_root_node;
        for (const std::string& key : path) {
            if (current_node_ptr->is_object() && current_node_ptr->get_object().contains(key)) {
                current_node_ptr = &current_node_ptr->get_object().at(key);
            } else return {};
        }
        auto dumped = current_node_ptr->dump();
        if (dumped) return dumped.value();
        return "";
    }

    // ========================================================================
    // Data Manipulation & Traversal (Identical to previous implementation)
    // ========================================================================

    void properties::merge(const properties& other_properties)
    {
        deep_merge(m_root_node, other_properties.m_root_node);
    }

    void properties::deep_merge(glz::json_t& destination, const glz::json_t& source)
    {
        if (source.is_null()) return;

        if (source.is_object() && destination.is_object()) {
            auto& destination_map = destination.get_object();
            for (const auto& [key, value] : source.get_object()) {
                if (value.is_null()) {
                    destination_map.erase(key);
                } else {
                    deep_merge(destination_map[key], value);
                }
            }
        } else {
            destination = source;
        }
    }

    std::expected<void, std::string> properties::move(const key_path& source_path, const key_path& destination_path)
    {
        if (source_path == destination_path || source_path.empty()) return {};

        glz::json_t* source_parent_node_ptr = &m_root_node;
        for (size_t i = 0; i < source_path.size() - 1; ++i) {
            if (!source_parent_node_ptr->is_object()) return std::unexpected("Source path not found");
            source_parent_node_ptr = &source_parent_node_ptr->get_object()[source_path[i]];
        }

        auto& source_object_map = source_parent_node_ptr->get_object();
        auto source_iterator = source_object_map.find(source_path.back());
        if (source_iterator == source_object_map.end()) return std::unexpected("Source path not found");

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
        return {};
    }

    std::expected<void, std::string> properties::rename(const key_path& path, const std::string& new_name)
    {
        if (path.empty() || new_name.empty()) return std::unexpected("Invalid path or name");
        key_path renamed_key_path = path;
        renamed_key_path.back() = new_name;
        auto res = move(path, renamed_key_path);
        if (!res) return std::unexpected(res.error());
        return {};
    }

    std::expected<void, std::string> properties::remove(const key_path& path)
    {
        if (path.empty()) return {};
        glz::json_t* current_node_ptr = &m_root_node;
        for (size_t i = 0; i < path.size() - 1; ++i) {
            if (!current_node_ptr->is_object()) return {};
            current_node_ptr = &current_node_ptr->get_object()[path[i]];
        return {};
        return {};
        return {};
        }
        if (current_node_ptr->is_object()) {
            current_node_ptr->get_object().erase(path.back());
        }
        return {};
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

    std::expected<properties, std::string> properties::get_subtree(const key_path& path) const
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
            key_list_result.reserve(current_node_ptr->get_object().size());
            for (const auto& [key, value] : current_node_ptr->get_object()) {
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
        if (!glz::write_json(current_node, serialized_json_value)) {
            callback(current_path, serialized_json_value);
        }

        if (current_node.is_object()) {
            for (const auto& [key, child_node] : current_node.get_object()) {
                current_path.push_back(key);
                walk(child_node, current_path, callback);
                current_path.pop_back();
            }
        }
    }

} // namespace sandbox