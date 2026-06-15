#include "properties.h"
#include <glaze/glaze.hpp>
#include <glaze/toml.hpp>
#include <glaze/yaml.hpp>
#include <stdexcept>

namespace sandbox::core {

    // Updated to glz::generic
    properties_t::properties_t() : m_data(glz::generic::object_t{}) {}

    properties_t::~properties_t() = default;

    void properties_t::load(std::string_view data, Format format) {
        glz::error_ctx ec{}; // Capture the error context

        switch (format) {
            case Format::JSON:
                ec = glz::read_json(m_data, data);
                break;
            case Format::BEVE:
                ec = glz::read_beve(m_data, data);
                break;
            case Format::TOML:
                ec = glz::read_toml(m_data, data);
                break;
            case Format::YAML:
                ec = glz::read_yaml(m_data, data);
                break;
            default:
                throw std::invalid_argument("Unsupported format provided to properties_t::load");
        }

        if (ec) {
            throw std::runtime_error("properties_t Load Error: " + glz::format_error(ec, data));
        }
    }

    std::string properties_t::dump(Format format) const {
        std::string serialized_output;
        glz::error_ctx ec{}; // Capture the error context

        switch (format) {
            case Format::JSON:
                ec = glz::write_json(m_data, serialized_output);
                break;
            case Format::BEVE:
                ec = glz::write_beve(m_data, serialized_output);
                break;
            case Format::TOML:
                ec = glz::write_toml(m_data, serialized_output);
                break;
            case Format::YAML:
                ec = glz::write_yaml(m_data, serialized_output);
                break;
            default:
                throw std::invalid_argument("Unsupported format provided to properties_t::dump");
        }

        // Properly handle the [[nodiscard]] error
        if (ec) {
            throw std::runtime_error("properties_t Dump Error: Serialization failed.");
        }

        return serialized_output;
    }

    void properties_t::clear(const path_t& path) {
        if (path.empty()) {
            // Updated to glz::generic
            m_data = glz::generic::object_t{};
            return;
        }

        path_t parent_path(path.begin(), path.end() - 1);
        const std::string& target_key = path.back();

        // Updated to glz::generic
        glz::generic* parent_node = detail::RetrieveOrCreateNode(m_data, parent_path);

        if (parent_node && parent_node->is_object()) {
            parent_node->get_object().erase(target_key);
        }
    }

    bool properties_t::has(const path_t& path) const {
        return detail::RetrieveNodeReadOnly(m_data, path) != nullptr;
    }

    properties_t::keys_t properties_t::keys(const path_t& path) const {
        keys_t extracted_keys;

        // Updated to glz::generic
        const glz::generic* target_node = detail::RetrieveNodeReadOnly(m_data, path);

        if (target_node && target_node->is_object()) {
            const auto& object_representation = target_node->get_object();
            extracted_keys.reserve(object_representation.size());

            for (const auto& [key_name, value_node] : object_representation) {
                extracted_keys.push_back(key_name);
            }
        }

        return extracted_keys;
    }

    void properties_t::merge(const path_t& path, const properties_t& other) {
        // Updated to glz::generic
        glz::generic* target_node = detail::RetrieveOrCreateNode(m_data, path);
        *target_node = other.m_data;
    }

    properties_t properties_t::sub(const path_t& path) const {
        properties_t extracted_properties;

        // Updated to glz::generic
        const glz::generic* target_node = detail::RetrieveNodeReadOnly(m_data, path);

        if (target_node) {
            extracted_properties.m_data = *target_node;
        }

        return extracted_properties;
    }

}