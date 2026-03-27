#include "../../include/sandbox/core/properties.h"
#include "../../include/sandbox/diagnostics/scoped_logger.h"
#include <glaze/json/write.hpp>
#include <glaze/json/read.hpp>
#include <fstream>

namespace sandbox
{
    void properties::load(const std::filesystem::path& file_path)
    {
        if (!std::filesystem::exists(file_path))
        {
            SANDBOX_LOG_ERROR("Properties: Failed to load. File does not exist: {}", file_path.string());
            return;
        }

        std::string content_buffer;
        std::ifstream input_stream{file_path, std::ios::binary | std::ios::ate};

        if (!input_stream)
        {
            SANDBOX_LOG_ERROR("Properties: Failed to open file stream for: {}", file_path.string());
            return;
        }

        const auto file_size = input_stream.tellg();
        content_buffer.resize(file_size);
        input_stream.seekg(0);
        input_stream.read(content_buffer.data(), file_size);

        auto error = glz::read_json(_data, content_buffer);
        if (error)
        {
            SANDBOX_LOG_ERROR("Properties: JSON Parse Error in {}. Error Code: {}", file_path.string(), static_cast<int>(error.ec));
        }
    }

    void properties::save(const std::filesystem::path& file_path) const
    {
        std::string output_buffer;
        auto error = glz::write_json(_data, output_buffer);

        if (error)
        {
            SANDBOX_LOG_ERROR("Properties: Serialization failed for save. Error Code: {}", static_cast<int>(error.ec));
            return;
        }

        std::ofstream output_stream{file_path};
        if (!output_stream)
        {
            SANDBOX_LOG_ERROR("Properties: Could not open file for writing: {}", file_path.string());
            return;
        }

        output_stream << output_buffer;
    }

    std::string properties::show() const
    {
        std::string output_buffer;
        auto error = glz::write_json(_data, output_buffer);
        if (error)
        {
            SANDBOX_LOG_ERROR("Properties: Failed to serialize tree for display.");
            return "{}";
        }
        return output_buffer;
    }

    void properties::clear()
    {
        _data = glz::json_t::object_t{};
    }

    void properties::merge(const properties& other_properties)
    {
        if (!other_properties._data.is_object() || !_data.is_object())
        {
            SANDBOX_LOG_WARN("Properties: Merge failed. One or both property sets are not objects.");
            return;
        }

        for (auto& [property_key, property_value] : other_properties._data.get_object())
        {
            _data.get_object()[property_key] = property_value;
        }
    }

    void properties::remove(const path& property_path)
    {
        if (property_path.empty()) return;

        auto* current_node_ptr = &_data;
        for (size_t index = 0; index < property_path.size() - 1; ++index)
        {
            const auto& segment = property_path[index];
            if (!current_node_ptr->is_object() || !current_node_ptr->get_object().contains(segment))
            {
                return; // Silently return if path doesn't exist
            }
            current_node_ptr = &current_node_ptr->get_object()[segment];
        }

        if (current_node_ptr->is_object())
        {
            current_node_ptr->get_object().erase(property_path.back());
        }
    }

    void properties::move(const path& source_path, const path& destination_path)
    {
        auto found_value = get<glz::json_t>(source_path);
        if (found_value)
        {
            set(destination_path, *found_value);
            remove(source_path);
        }
        else
        {
            SANDBOX_LOG_WARN("Properties: Move failed. Source path does not exist.");
        }
    }

    void properties::rename(const path& old_path, const path& new_path)
    {
        move(old_path, new_path);
    }

    bool properties::exists(const path& property_path) const
    {
        const auto* current_node_ptr = &_data;
        for (const auto& path_segment : property_path)
        {
            if (!current_node_ptr->is_object() || !current_node_ptr->get_object().contains(path_segment))
            {
                return false;
            }
            current_node_ptr = &current_node_ptr->get_object().at(path_segment);
        }
        return true;
    }

    properties properties::sub_properties(const path& property_path) const
    {
        properties sub_tree;
        auto found_value = get<glz::json_t>(property_path);
        if (found_value)
        {
            sub_tree._data = *found_value;
        }
        else
        {
            SANDBOX_LOG_WARN("Properties: Failed to extract sub-properties for missing path.");
        }
        return sub_tree;
    }

    properties::keys properties::get_keys(const path& property_path) const
    {
        const auto* current_node_ptr = &_data;
        for (const auto& path_segment : property_path)
        {
            if (!current_node_ptr->is_object() || !current_node_ptr->get_object().contains(path_segment))
            {
                return {};
            }
            current_node_ptr = &current_node_ptr->get_object().at(path_segment);
        }

        if (!current_node_ptr->is_object()) return {};

        keys key_list;
        for (const auto& [entry_key, entry_value] : current_node_ptr->get_object())
        {
            key_list.push_back(entry_key);
        }
        return key_list;
    }

    void properties::for_each(const std::function<void(const path&, const std::string&)>& callback) const
    {
        path current_traversal_path;
        _walk_tree(_data, current_traversal_path, callback);
    }

    void properties::_walk_tree(const glz::json_t& current_node, path& traversal_path, const std::function<void(const path&, const std::string&)>& callback) const
    {
        if (current_node.is_object())
        {
            for (auto& [node_key, node_value] : current_node.get_object())
            {
                traversal_path.push_back(node_key);
                _walk_tree(node_value, traversal_path, callback);
                traversal_path.pop_back();
            }
        }
        else
        {
            std::string serialized_json;
            auto error = glz::write_json(current_node, serialized_json);
            if (!error)
            {
                callback(traversal_path, serialized_json);
            }
            else
            {
                SANDBOX_LOG_ERROR("Properties: Failed to serialize node during tree walk.");
            }
        }
    }
}