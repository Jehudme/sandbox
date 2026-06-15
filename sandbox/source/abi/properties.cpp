#include "sandbox/core/properties.h"
#include "core/properties.h"
#include <cstring>
#include <cstdlib>
#include <exception>

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

    char** sandbox_properties_keys(const sandbox_properties_t* props, const char* path_str, size_t* out_count) {
        if (!props || !out_count) return nullptr;

        auto cpp_keys = cast(props)->keys(parse_path(path_str));
        *out_count = cpp_keys.size();

        if (cpp_keys.empty()) return nullptr;

        char** c_keys = (char**)malloc(sizeof(char*) * cpp_keys.size());
        for (size_t i = 0; i < cpp_keys.size(); ++i) {
            c_keys[i] = (char*)malloc(cpp_keys[i].size() + 1);
            std::memcpy(c_keys[i], cpp_keys[i].c_str(), cpp_keys[i].size() + 1);
        }
        return c_keys;
    }

    void sandbox_properties_free_keys(char** keys, size_t count) {
        if (keys) {
            for (size_t i = 0; i < count; ++i) {
                free(keys[i]);
            }
            free(keys);
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

    const char* sandbox_properties_get_string(const sandbox_properties_t* props, const char* path_str) {
        if (!props) return nullptr;
        if (auto val = cast(props)->get<std::string>(parse_path(path_str))) {
            // Use thread_local to safely pass string pointer across ABI without requiring the caller to free it
            thread_local std::string scratchpad;
            scratchpad = std::move(*val);
            return scratchpad.c_str();
        }

        return nullptr;
    }

    // --- SETTERS ---

    void sandbox_properties_set_int64(sandbox_properties_t* props, const char* path_str, int64_t val) {
        if (props) cast(props)->set<int64_t>(parse_path(path_str), val);
    }

    void sandbox_properties_set_double(sandbox_properties_t* props, const char* path_str, double val) {
        if (props) cast(props)->set<double>(parse_path(path_str), val);
    }

    void sandbox_properties_set_bool(sandbox_properties_t* props, const char* path_str, bool val) {
        if (props) cast(props)->set<bool>(parse_path(path_str), val);
    }

    void sandbox_properties_set_string(sandbox_properties_t* props, const char* path_str, const char* val) {
        if (props && val) {
            std::string cpp_val(val);
            cast(props)->set<std::string>(parse_path(path_str), cpp_val);
        }
    }
}