#include "sandbox/abi/properties.h"
#include "core/properties.h"
#include <cstring>
#include <cstdlib>
#include <exception>
#include <iostream>

using namespace sandbox::core;

// Helper to safely cast the opaque C pointer back to the real C++ class
static properties_t* cast(sandbox_properties_t* props) {
    return reinterpret_cast<properties_t*>(props);
}

static const properties_t* cast(const sandbox_properties_t* props) {
    return reinterpret_cast<const properties_t*>(props);
}

#include <string_view>

// Helper to build the C++ Path vector from a string format "a/b/c"
static properties_t::path_t parse_path(const char* path_str) {
    properties_t::path_t cpp_path;
    if (!path_str) return cpp_path;
    
    std::string_view view(path_str);
    if (view.empty()) return cpp_path;

    size_t start = 0;
    while (start < view.length()) {
        size_t end = view.find('/', start);
        if (end == std::string_view::npos) {
            cpp_path.emplace_back(view.substr(start));
            break;
        }
        cpp_path.emplace_back(view.substr(start, end - start));
        start = end + 1;
    }
    return cpp_path;
}

static properties_t::Format map_format(sandbox_properties_format_t fmt) {
    switch (fmt) {
        case SANDBOX_FORMAT_JSON: return properties_t::Format::JSON;
        case SANDBOX_FORMAT_BEVE: return properties_t::Format::BEVE;
        case SANDBOX_FORMAT_TOML: return properties_t::Format::TOML;
        case SANDBOX_FORMAT_YAML: return properties_t::Format::YAML;
        default: return properties_t::Format::JSON;
    }
}

extern "C" {

    sandbox_properties_t* sandbox_properties_create(void) {
        return reinterpret_cast<sandbox_properties_t*>(new properties_t());
    }

    void sandbox_properties_destroy(sandbox_properties_t* props) {
        if (props) delete cast(props);
    }

    bool sandbox_properties_load(sandbox_properties_t* props, const char* data, size_t data_length, sandbox_properties_format_t format) {
        if (!props || !data) return false;
        try {
            std::string_view view(data, data_length);
            cast(props)->load(view, map_format(format));
            return true;
        } catch (...) {
            // Catching all exceptions ensures the C plugin doesn't abruptly crash the engine
            return false;
        }
    }

    char* sandbox_properties_dump(const sandbox_properties_t* props, sandbox_properties_format_t format) {
        if (!props) return nullptr;
        try {
            std::string result = cast(props)->dump(map_format(format));
            // Duplicate string to heap for C lifecycle management
            char* c_str = (char*)malloc(result.size() + 1);
            if (c_str) std::memcpy(c_str, result.c_str(), result.size() + 1);
            return c_str;
        } catch (...) {
            return nullptr;
        }
    }

    void sandbox_properties_free_string(char* str) {
        if (str) free(str);
    }

    void sandbox_properties_clear(sandbox_properties_t* props, const char* path_str) {
        if (props) cast(props)->clear(parse_path(path_str));
    }

    bool sandbox_properties_has(const sandbox_properties_t* props, const char* path_str) {
        if (!props) return false;
        return cast(props)->has(parse_path(path_str));
    }

    void sandbox_properties_keys(const sandbox_properties_t* props, const char* path_str, void (*callback)(const char* key, void* ctx), void* ctx) {
        if (!props || !callback) return;

        auto cpp_keys = cast(props)->keys(parse_path(path_str));
        for (const auto& k : cpp_keys) {
            callback(k.c_str(), ctx);
        }
    }

    void sandbox_properties_merge(sandbox_properties_t* props, const char* path_str, const sandbox_properties_t* other) {
        if (props && other) {
            cast(props)->merge(parse_path(path_str), *cast(other));
        }
    }

    sandbox_properties_t* sandbox_properties_sub(const sandbox_properties_t* props, const char* path_str) {
        if (!props) return nullptr;
        properties_t sub_props = cast(props)->sub(parse_path(path_str));
        return reinterpret_cast<sandbox_properties_t*>(new properties_t(std::move(sub_props)));
    }

    // --- GETTERS ---

    bool sandbox_properties_get_int64(const sandbox_properties_t* props, const char* path_str, int64_t* out_val) {
        if (!props || !out_val) return false;
        if (auto val = cast(props)->get<int64_t>(parse_path(path_str))) {
            *out_val = *val;
            return true;
        }
        return false;
    }

    bool sandbox_properties_get_uint64(const sandbox_properties_t* props, const char* path_str, uint64_t* out_val) {
        if (!props || !out_val) return false;
        if (auto val = cast(props)->get<uint64_t>(parse_path(path_str))) {
            *out_val = *val;
            return true;
        }
        return false;
    }

    bool sandbox_properties_get_double(const sandbox_properties_t* props, const char* path_str, double* out_val) {
        if (!props || !out_val) return false;
        if (auto val = cast(props)->get<double>(parse_path(path_str))) {
            *out_val = *val;
            return true;
        }
        return false;
    }

    bool sandbox_properties_get_bool(const sandbox_properties_t* props, const char* path_str, bool* out_val) {
        if (!props || !out_val) return false;
        if (auto val = cast(props)->get<bool>(parse_path(path_str))) {
            *out_val = *val;
            return true;
        }
        return false;
    }

    void sandbox_properties_read_string(const sandbox_properties_t* props, const char* path_str, void (*callback)(const char* value, void* user_data), void* user_data) {
        if (!callback) return;
        if (!props) {
            callback(nullptr, user_data);
            return;
        }
        if (auto val = cast(props)->get<std::string>(parse_path(path_str))) {
            callback(val->c_str(), user_data);
        } else {
            callback(nullptr, user_data);
        }
    }

    void sandbox_properties_read_int64_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(int64_t value, void* user_data), void* user_data) {
        if (!callback || !props) return;
        if (auto arr = cast(props)->get<std::vector<int64_t>>(parse_path(path_str))) {
            for (const auto& val : *arr) callback(val, user_data);
        }
    }

    void sandbox_properties_read_uint64_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(uint64_t value, void* user_data), void* user_data) {
        if (!callback || !props) return;
        if (auto arr = cast(props)->get<std::vector<uint64_t>>(parse_path(path_str))) {
            for (const auto& val : *arr) callback(val, user_data);
        }
    }

    void sandbox_properties_read_double_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(double value, void* user_data), void* user_data) {
        if (!callback || !props) return;
        if (auto arr = cast(props)->get<std::vector<double>>(parse_path(path_str))) {
            for (const auto& val : *arr) callback(val, user_data);
        }
    }

    void sandbox_properties_read_bool_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(bool value, void* user_data), void* user_data) {
        if (!callback || !props) return;
        if (auto arr = cast(props)->get<std::vector<bool>>(parse_path(path_str))) {
            for (auto val : *arr) callback(val, user_data);
        }
    }

    void sandbox_properties_read_string_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(const char* value, void* user_data), void* user_data) {
        if (!callback || !props) return;
        if (auto arr = cast(props)->get<std::vector<std::string>>(parse_path(path_str))) {
            for (const auto& val : *arr) {
                callback(val.c_str(), user_data);
            }
        }
    }

    // --- SETTERS ---

    void sandbox_properties_set_int64(sandbox_properties_t* props, const char* path_str, int64_t val) {
        if (props) cast(props)->set<int64_t>(parse_path(path_str), val);
    }

    void sandbox_properties_set_uint64(sandbox_properties_t* props, const char* path_str, uint64_t val) {
        if (props) cast(props)->set<uint64_t>(parse_path(path_str), val);
    }

    void sandbox_properties_set_double(sandbox_properties_t* props, const char* path_str, double val) {
        if (props) cast(props)->set<double>(parse_path(path_str), val);
    }

    void sandbox_properties_set_bool(sandbox_properties_t* props, const char* path_str, bool val) {
        if (props) cast(props)->set<bool>(parse_path(path_str), val);
    }

    void sandbox_properties_set_string(sandbox_properties_t* props, const char* path_str, const char* val) {
        if (!props || !val) return;
        cast(props)->set<std::string>(parse_path(path_str), std::string(val));
    }

    void sandbox_properties_set_int64_array(sandbox_properties_t* props, const char* path_str, const int64_t* values, size_t count) {
        if (!props || (!values && count > 0)) return;
        std::vector<int64_t> arr(values, values + count);
        cast(props)->set<std::vector<int64_t>>(parse_path(path_str), std::move(arr));
    }

    void sandbox_properties_set_uint64_array(sandbox_properties_t* props, const char* path_str, const uint64_t* values, size_t count) {
        if (!props || (!values && count > 0)) return;
        std::vector<uint64_t> arr(values, values + count);
        cast(props)->set<std::vector<uint64_t>>(parse_path(path_str), std::move(arr));
    }

    void sandbox_properties_set_double_array(sandbox_properties_t* props, const char* path_str, const double* values, size_t count) {
        if (!props || (!values && count > 0)) return;
        std::vector<double> arr(values, values + count);
        cast(props)->set<std::vector<double>>(parse_path(path_str), std::move(arr));
    }

    void sandbox_properties_set_bool_array(sandbox_properties_t* props, const char* path_str, const bool* values, size_t count) {
        if (!props || (!values && count > 0)) return;
        std::vector<bool> arr(values, values + count);
        cast(props)->set<std::vector<bool>>(parse_path(path_str), std::move(arr));
    }

    void sandbox_properties_set_string_array(sandbox_properties_t* props, const char* path_str, const char** values, size_t count) {
        if (!props || (!values && count > 0)) return;
        std::vector<std::string> arr;
        arr.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            if (values[i]) arr.emplace_back(values[i]);
        }
        cast(props)->set<std::vector<std::string>>(parse_path(path_str), std::move(arr));
    }
}