#include <glaze/glaze.hpp>
#include "../diagnostics/scoped_logger.h"

namespace sandbox
{
    template<typename target_type>
    std::optional<target_type> properties::get(const path& property_path) const
    {
        const auto* current_node_ptr = &_data;
        for (const auto& path_segment : property_path)
        {
            if (!current_node_ptr->is_object() || !current_node_ptr->get_object().contains(path_segment))
            {
                SANDBOX_LOG_WARN("Properties: Path segment '{}' not found in tree.", path_segment);
                return std::nullopt;
            }
            current_node_ptr = &current_node_ptr->get_object().at(path_segment);
        }

        target_type result_value{};
        auto error = glz::read<glz::opts{}>(result_value, *current_node_ptr);
        if (error)
        {
            SANDBOX_LOG_ERROR("Properties: Type conversion failed for path. Glaze Error: {}", static_cast<int>(error.ec));
            return std::nullopt;
        }

        return result_value;
    }

    template<typename target_type>
    void properties::set(const path& property_path, const target_type& value_to_set)
    {
        auto* current_node_ptr = &_data;
        for (const auto& path_segment : property_path)
        {
            if (!current_node_ptr->is_object())
            {
                *current_node_ptr = glz::json_t::object_t{};
            }
            current_node_ptr = &current_node_ptr->get_object()[path_segment];
        }

        *current_node_ptr = value_to_set;
    }
}