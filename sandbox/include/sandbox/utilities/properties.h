#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <cstddef> // Required for std::byte
#include <glaze/json/json_t.hpp>
#include "sandbox/core/platform.h"

namespace sandbox
{
    class SANDBOX_API properties
    {
    public:
        using key_path = std::vector<std::string>;
        using key_list = std::vector<std::string>;
        using visitor_callback = std::function<void(const key_path& path, const std::string& json_value)>;

        properties(std::string_view json_string);
        properties(const std::vector<std::byte>& byte_data);

        properties() = default;
        ~properties() = default;

        properties& operator=(const properties&) = default;
        properties(const properties&) = default;

        void load_from_string(std::string_view json_string);
        void load_from_bytes(const std::vector<std::byte>& byte_data);
        std::string save_to_string(const key_path& path = {}) const;

        static properties parse(std::string_view json_string);
        static properties parse(const std::vector<std::byte>& byte_data);

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

#include "detail/properties.inl"