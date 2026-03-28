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
        using key_path = std::vector<std::string>;
        using key_list = std::vector<std::string>;
        using visitor_callback = std::function<void(const key_path& path, const std::string& json_value)>;

        properties() = default;

        void load_from_file(const std::filesystem::path& file_path);
        void save_to_file(const std::filesystem::path& file_path) const;
        std::string to_json_string(const key_path& path = {}) const;


        void merge(const properties& other_properties);
        void move(const key_path& source_path, const key_path& destination_path);
        void rename(const key_path& path, const std::string& new_name);
        void remove(const key_path& path);
        void clear() noexcept;

        bool contains(const key_path& path) const;
        properties get_subtree(const key_path& path) const;
        key_list list_keys(const key_path& path = {}) const;

        void traverse(const visitor_callback& callback) const;

        template<typename target_type>
        std::optional<target_type> get(const key_path& path) const;

        template<typename target_type>
        void set(const key_path& path, const target_type& value_to_set);

    private:
        void walk(const glz::json_t& current_node, key_path& current_path, const visitor_callback& callback) const;
        void deep_merge(glz::json_t& destination, const glz::json_t& source);

        glz::json_t m_root_node;
    };
}

#include "properties.inl"
