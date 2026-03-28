#include <glaze/glaze.hpp>

namespace sandbox
{
    template<typename target_type>
    std::optional<target_type> properties::get(const key_path& path) const
    {
        const glz::json_t* current_node_ptr = &m_root_node;

        for (const std::string& key : path) {
            if (current_node_ptr->is_object() && current_node_ptr->get_object().contains(key)) {
                current_node_ptr = &current_node_ptr->get_object().at(key);
            } else {
                return std::nullopt;
            }
        }

        target_type deserialized_value{};
        const auto error_code = glz::read<glz::opts{}>(deserialized_value, *current_node_ptr);

        if (!error_code) {
            return deserialized_value;
        }
        return std::nullopt;
    }

    template<typename target_type>
    void properties::set(const key_path& path, const target_type& value_to_set)
    {
        glz::json_t* current_node_ptr = &m_root_node;

        for (const std::string& key : path) {
            if (!current_node_ptr->is_object()) {
                *current_node_ptr = glz::json_t::object_t{};
            }
            current_node_ptr = &current_node_ptr->get_object()[key];
        }

        std::string serialization_buffer;
        glz::write_json(value_to_set, serialization_buffer);
        glz::read_json(*current_node_ptr, serialization_buffer);
    }
}