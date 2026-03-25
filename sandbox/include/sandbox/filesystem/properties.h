#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <glaze/json/json_t.hpp>

namespace sandbox
{
    class properties
    {
    public:
        using path = std::vector<std::string>;
        using keys = std::vector<std::string>;

        properties() = default;

        void load(const std::filesystem::path& file_path);
        void save(const std::filesystem::path& file_path) const;
        [[nodiscard]] std::string show() const;

        void merge(const properties& other_properties);
        void move(const path& source_path, const path& destination_path);
        void rename(const path& old_path, const path& new_path);
        void remove(const path& property_path);
        void clear();

        [[nodiscard]] bool exists(const path& property_path) const;
        [[nodiscard]] properties sub_properties(const path& property_path) const;
        [[nodiscard]] keys get_keys(const path& property_path) const;
        void for_each(const std::function<void(const path& current_path, const std::string& json_value)>& callback) const;

        template<typename target_type>
        std::optional<target_type> get(const path& property_path) const;

        template<typename target_type>
        void set(const path& property_path, const target_type& value_to_set);

    private:
        glz::json_t _data;

        void _walk_tree(const glz::json_t& current_node, path& traversal_path, const std::function<void(const path&, const std::string&)>& callback) const;
    };
}

#include "properties.inl"
