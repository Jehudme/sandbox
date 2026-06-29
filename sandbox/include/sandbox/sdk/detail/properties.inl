#pragma once
#include "../properties.hpp"
#include <stdexcept>

namespace sandbox {

    inline properties::properties() {
        sandbox_properties_create(&m_handle);
    }

    inline properties::~properties() {
        sandbox_properties_destroy(&m_handle);
    }

    inline properties::properties(properties&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle.token = 0;
    }

    inline properties& properties::operator=(properties&& other) noexcept {
        if (this != &other) {
            sandbox_properties_destroy(&m_handle);
            m_handle = other.m_handle;
            other.m_handle.token = 0;
        }
        return *this;
    }

    inline properties::properties(sandbox_handle_t raw) : m_handle(raw) {}

    inline bool properties::load(const std::string& data, Format format) {
        sandbox_properties_format_t fmt = SANDBOX_FORMAT_JSON;
        switch (format) {
            case Format::JSON: fmt = SANDBOX_FORMAT_JSON; break;
            case Format::BEVE: fmt = SANDBOX_FORMAT_BEVE; break;
            case Format::TOML: fmt = SANDBOX_FORMAT_TOML; break;
            case Format::YAML: fmt = SANDBOX_FORMAT_YAML; break;
        }
        return sandbox_properties_load(m_handle, data.c_str(), data.size(), fmt);
    }

    inline std::string properties::dump(Format format) const {
        sandbox_properties_format_t fmt = SANDBOX_FORMAT_JSON;
        switch (format) {
            case Format::JSON: fmt = SANDBOX_FORMAT_JSON; break;
            case Format::BEVE: fmt = SANDBOX_FORMAT_BEVE; break;
            case Format::TOML: fmt = SANDBOX_FORMAT_TOML; break;
            case Format::YAML: fmt = SANDBOX_FORMAT_YAML; break;
        }
        char* c_str = sandbox_properties_dump(m_handle, fmt);
        if (!c_str) return "";
        std::string result(c_str);
        sandbox_properties_free_string(c_str);
        return result;
    }

    inline void properties::clear(const std::string& path) {
        sandbox_properties_clear(m_handle, path.c_str());
    }

    inline bool properties::has(const std::string& path) const {
        return sandbox_properties_has(m_handle, path.c_str());
    }

    inline std::vector<std::string> properties::keys(const std::string& path) const {
        std::vector<std::string> result;
        sandbox_properties_keys(m_handle, path.c_str(), [](const char* key, void* ctx) {
            auto* vec = static_cast<std::vector<std::string>*>(ctx);
            vec->emplace_back(key);
        }, &result);
        return result;
    }

    inline void properties::merge(const std::string& path, const properties& other) {
        sandbox_properties_merge(m_handle, path.c_str(), other.get_raw());
    }

    inline properties properties::sub(const std::string& path) const {
        sandbox_handle_t raw_sub{0};
        sandbox_properties_sub(m_handle, path.c_str(), &raw_sub);
        return properties(raw_sub); // Taking ownership
    }

    template <typename T>
    inline bool properties::get(const std::string& path, T& out_val) const {
        if constexpr (std::is_same_v<T, std::string>) {
            bool found = false;
            struct Ctx { std::string* str; bool* found; } ctx = { &out_val, &found };
            sandbox_properties_read_string(m_handle, path.c_str(), [](const char* value, void* user_data) {
                auto* c = static_cast<Ctx*>(user_data);
                if (value) { *c->str = value; *c->found = true; }
            }, &ctx);
            return found;
        } else if constexpr (std::is_same_v<T, bool>) {
            return sandbox_properties_get_bool(m_handle, path.c_str(), &out_val);
        } else if constexpr (std::is_floating_point_v<T>) {
            double val;
            if (sandbox_properties_get_double(m_handle, path.c_str(), &val)) {
                out_val = static_cast<T>(val);
                return true;
            }
            return false;
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_signed_v<T>) {
                int64_t val;
                if (sandbox_properties_get_int64(m_handle, path.c_str(), &val)) {
                    out_val = static_cast<T>(val);
                    return true;
                }
            } else {
                uint64_t val;
                if (sandbox_properties_get_uint64(m_handle, path.c_str(), &val)) {
                    out_val = static_cast<T>(val);
                    return true;
                }
            }
            return false;
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type for properties::get");
        }
    }

    template <typename T>
    inline std::optional<T> properties::get(const std::string& path) const {
        T val;
        if (get(path, val)) return val;
        return std::nullopt;
    }

    template <typename T>
    inline bool properties::get_array(const std::string& path, std::vector<T>& out_val) const {
        out_val.clear();
        if (!sandbox_properties_has(m_handle, path.c_str())) return false;

        if constexpr (std::is_same_v<T, std::string>) {
            sandbox_properties_read_string_array(m_handle, path.c_str(), [](const char* value, void* user_data) {
                auto* vec = static_cast<std::vector<std::string>*>(user_data);
                if (value) vec->emplace_back(value);
            }, &out_val);
        } else if constexpr (std::is_same_v<T, bool>) {
            sandbox_properties_read_bool_array(m_handle, path.c_str(), [](bool value, void* user_data) {
                static_cast<std::vector<bool>*>(user_data)->push_back(value);
            }, &out_val);
        } else if constexpr (std::is_floating_point_v<T>) {
            sandbox_properties_read_double_array(m_handle, path.c_str(), [](double value, void* user_data) {
                static_cast<std::vector<T>*>(user_data)->push_back(static_cast<T>(value));
            }, &out_val);
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_signed_v<T>) {
                sandbox_properties_read_int64_array(m_handle, path.c_str(), [](int64_t value, void* user_data) {
                    static_cast<std::vector<T>*>(user_data)->push_back(static_cast<T>(value));
                }, &out_val);
            } else {
                sandbox_properties_read_uint64_array(m_handle, path.c_str(), [](uint64_t value, void* user_data) {
                    static_cast<std::vector<T>*>(user_data)->push_back(static_cast<T>(value));
                }, &out_val);
            }
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type for properties::get_array");
        }
        return true;
    }

    template <typename T>
    inline void properties::set(const std::string& path, const T& val) {
        if constexpr (std::is_same_v<T, std::string>) {
            sandbox_properties_set_string(m_handle, path.c_str(), val.c_str());
        } else if constexpr (std::is_convertible_v<T, const char*>) {
            sandbox_properties_set_string(m_handle, path.c_str(), static_cast<const char*>(val));
        } else if constexpr (std::is_same_v<T, bool>) {
            sandbox_properties_set_bool(m_handle, path.c_str(), val);
        } else if constexpr (std::is_floating_point_v<T>) {
            sandbox_properties_set_double(m_handle, path.c_str(), static_cast<double>(val));
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_signed_v<T>) {
                sandbox_properties_set_int64(m_handle, path.c_str(), static_cast<int64_t>(val));
            } else {
                sandbox_properties_set_uint64(m_handle, path.c_str(), static_cast<uint64_t>(val));
            }
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type for properties::set");
        }
    }

    template <typename T>
    inline void properties::set_array(const std::string& path, const std::vector<T>& values) {
        if constexpr (std::is_same_v<T, std::string>) {
            std::vector<const char*> c_vals;
            c_vals.reserve(values.size());
            for (const auto& v : values) c_vals.push_back(v.c_str());
            sandbox_properties_set_string_array(m_handle, path.c_str(), c_vals.data(), c_vals.size());
        } else if constexpr (std::is_same_v<T, bool>) {
            std::vector<bool> bools(values.begin(), values.end());
            // std::vector<bool> is specialized, so we might need a workaround for C array
            bool* arr = new bool[bools.size()];
            for (size_t i = 0; i < bools.size(); ++i) arr[i] = bools[i];
            sandbox_properties_set_bool_array(m_handle, path.c_str(), arr, bools.size());
            delete[] arr;
        } else if constexpr (std::is_floating_point_v<T>) {
            std::vector<double> arr(values.begin(), values.end());
            sandbox_properties_set_double_array(m_handle, path.c_str(), arr.data(), arr.size());
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_signed_v<T>) {
                std::vector<int64_t> arr(values.begin(), values.end());
                sandbox_properties_set_int64_array(m_handle, path.c_str(), arr.data(), arr.size());
            } else {
                std::vector<uint64_t> arr(values.begin(), values.end());
                sandbox_properties_set_uint64_array(m_handle, path.c_str(), arr.data(), arr.size());
            }
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type for properties::set_array");
        }
    }

}
