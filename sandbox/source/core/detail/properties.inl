#pragma once

#include <type_traits>
#include <vector>
#include <string>
#include <optional>

namespace sandbox::core::detail {

    inline const glz::generic* RetrieveNodeReadOnly(const glz::generic& root_node, const std::vector<std::string>& search_path) {
        const glz::generic* current_iterator = &root_node;

        for (const std::string& path_key : search_path) {
            if (!current_iterator->is_object()) {
                return nullptr;
            }

            const auto& object_representation = current_iterator->get_object();
            auto found_iterator = object_representation.find(path_key);

            if (found_iterator == object_representation.end()) {
                return nullptr;
            }

            current_iterator = &found_iterator->second;
        }

        return current_iterator;
    }

    inline glz::generic* RetrieveOrCreateNode(glz::generic& root_node, const std::vector<std::string>& search_path) {
        glz::generic* current_iterator = &root_node;

        for (const std::string& path_key : search_path) {
            if (!current_iterator->is_object()) {
                *current_iterator = glz::generic::object_t{};
            }

            auto& object_representation = current_iterator->get_object();
            current_iterator = &object_representation[path_key];
        }

        return current_iterator;
    }

}

namespace sandbox::core {

    template <typename Type>
    std::optional<Type> Properties::get(const Path& path) const {
        const glz::generic* target_node = detail::RetrieveNodeReadOnly(m_data, path);

        if (!target_node) {
            return std::nullopt;
        }

        // Hardcoding explicit types guarantees the std::variant static_assert NEVER fires.
        if constexpr (std::is_arithmetic_v<Type> && !std::is_same_v<Type, bool>) {

            if (const double* val = target_node->template get_if<double>()) {
                return static_cast<Type>(*val);
            }

        } else if constexpr (std::is_same_v<Type, bool>) {

            if (const bool* val = target_node->template get_if<bool>()) {
                return *val;
            }

        } else if constexpr (std::is_same_v<Type, std::string>) {

            if (const std::string* val = target_node->template get_if<std::string>()) {
                return *val;
            }

        }

        return std::nullopt;
    }

    template <typename Type>
    void Properties::set(const Path& path, Type value) {
        glz::generic* target_node = detail::RetrieveOrCreateNode(m_data, path);

        // Ensure values are cast correctly into Glaze's expected memory layout
        if constexpr (std::is_arithmetic_v<Type> && !std::is_same_v<Type, bool>) {
            *target_node = static_cast<double>(value);
        } else if constexpr (std::is_same_v<Type, bool>) {
            *target_node = static_cast<bool>(value);
        } else {
            *target_node = value;
        }
    }

}