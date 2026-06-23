#pragma once
#include "../properties.hpp"
#include <stdexcept>

namespace sandbox {

    inline properties::properties() {
        m_props = sandbox_properties_create();
    }

    inline properties::~properties() {
        sandbox_properties_destroy(m_props);
    }

    inline properties::properties(properties&& other) noexcept : m_props(other.m_props) {
        other.m_props = nullptr;
    }

    inline properties& properties::operator=(properties&& other) noexcept {
        if (this != &other) {
            sandbox_properties_destroy(m_props);
            m_props = other.m_props;
            other.m_props = nullptr;
        }
        return *this;
    }

    inline properties::properties(sandbox_properties_t* raw) : m_props(raw) {}

    inline bool properties::load(const std::string& data, Format format) {
        sandbox_properties_format_t fmt = SANDBOX_FORMAT_JSON;
        switch (format) {
            case Format::JSON: fmt = SANDBOX_FORMAT_JSON; break;
            case Format::BEVE: fmt = SANDBOX_FORMAT_BEVE; break;
            case Format::TOML: fmt = SANDBOX_FORMAT_TOML; break;
            case Format::YAML: fmt = SANDBOX_FORMAT_YAML; break;
        }
        return sandbox_properties_load(m_props, data.c_str(), data.size(), fmt);
    }

    inline std::string properties::dump(Format format) const {
        sandbox_properties_format_t fmt = SANDBOX_FORMAT_JSON;
        switch (format) {
            case Format::JSON: fmt = SANDBOX_FORMAT_JSON; break;
            case Format::BEVE: fmt = SANDBOX_FORMAT_BEVE; break;
            case Format::TOML: fmt = SANDBOX_FORMAT_TOML; break;
            case Format::YAML: fmt = SANDBOX_FORMAT_YAML; break;
        }
        char* c_str = sandbox_properties_dump(m_props, fmt);
        if (!c_str) return "";
        std::string result(c_str);
        sandbox_properties_free_string(c_str);
        return result;
    }

    inline void properties::clear(const std::string& path) {
        sandbox_properties_clear(m_props, path.c_str());
    }

    inline bool properties::has(const std::string& path) const {
        return sandbox_properties_has(m_props, path.c_str());
    }

    inline std::vector<std::string> properties::keys(const std::string& path) const {
        std::vector<std::string> result;
        sandbox_properties_keys(m_props, path.c_str(), [](const char* key, void* ctx) {
            auto* vec = static_cast<std::vector<std::string>*>(ctx);
            vec->emplace_back(key);
        }, &result);
        return result;
    }

    inline void properties::merge(const std::string& path, const properties& other) {
        sandbox_properties_merge(m_props, path.c_str(), other.get_raw());
    }

    inline properties properties::sub(const std::string& path) const {
        sandbox_properties_t* raw_sub = sandbox_properties_sub(m_props, path.c_str());
        return properties(raw_sub); // Taking ownership
    }

    inline bool properties::get_int64(const std::string& path, int64_t& out_val) const {
        return sandbox_properties_get_int64(m_props, path.c_str(), &out_val);
    }

    inline bool properties::get_double(const std::string& path, double& out_val) const {
        return sandbox_properties_get_double(m_props, path.c_str(), &out_val);
    }

    inline bool properties::get_bool(const std::string& path, bool& out_val) const {
        return sandbox_properties_get_bool(m_props, path.c_str(), &out_val);
    }

    inline bool properties::get_string(const std::string& path, std::string& out_val) const {
        bool found = false;
        struct Ctx { std::string* str; bool* found; } ctx = { &out_val, &found };
        
        sandbox_properties_read_string(m_props, path.c_str(), [](const char* value, void* user_data) {
            auto* c = static_cast<Ctx*>(user_data);
            if (value) {
                *c->str = value;
                *c->found = true;
            }
        }, &ctx);
        return found;
    }

    inline bool properties::get_string_array(const std::string& path, std::vector<std::string>& out_val) const {
        out_val.clear();
        bool found = false; // We set found to true if the array exists, even if empty
        // We can check if it exists first
        if (!sandbox_properties_has(m_props, path.c_str())) return false;
        sandbox_properties_read_string_array(m_props, path.c_str(), [](const char* value, void* user_data) {
            auto* vec = static_cast<std::vector<std::string>*>(user_data);
            if (value) vec->emplace_back(value);
        }, &out_val);
        return true;
    }

    inline void properties::set_int64(const std::string& path, int64_t val) {
        sandbox_properties_set_int64(m_props, path.c_str(), val);
    }

    inline void properties::set_double(const std::string& path, double val) {
        sandbox_properties_set_double(m_props, path.c_str(), val);
    }

    inline void properties::set_bool(const std::string& path, bool val) {
        sandbox_properties_set_bool(m_props, path.c_str(), val);
    }

    inline void properties::set_string(const std::string& path, const std::string& val) {
        sandbox_properties_set_string(m_props, path.c_str(), val.c_str());
    }

    inline void properties::set_string_array(const std::string& path, const std::vector<std::string>& values) {
        std::vector<const char*> c_vals;
        c_vals.reserve(values.size());
        for (const auto& v : values) {
            c_vals.push_back(v.c_str());
        }
        sandbox_properties_set_string_array(m_props, path.c_str(), c_vals.data(), c_vals.size());
    }

}
