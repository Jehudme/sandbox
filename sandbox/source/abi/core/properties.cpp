#include "sandbox/core/properties.h"
#include "core/properties.h"
#include <cstring>
#include <cstdlib>
#include <exception>

using namespace sandbox::core;

// Helper to safely cast the opaque C pointer back to the real C++ class
static Properties* cast(sandbox_properties_t* props) {
    return reinterpret_cast<Properties*>(props);
}

static const Properties* cast(const sandbox_properties_t* props) {
    return reinterpret_cast<const Properties*>(props);
}

// Helper to build the C++ Path vector from the flat C array
static Properties::Path make_path(const char** path, size_t path_len) {
    Properties::Path cpp_path;
    if (path && path_len > 0) {
        cpp_path.reserve(path_len);
        for (size_t i = 0; i < path_len; ++i) {
            if (path[i]) cpp_path.emplace_back(path[i]);
        }
    }
    return cpp_path;
}

static Properties::Format map_format(sandbox_properties_format_t fmt) {
    switch (fmt) {
        case SANDBOX_FORMAT_JSON: return Properties::Format::JSON;
        case SANDBOX_FORMAT_BEVE: return Properties::Format::BEVE;
        case SANDBOX_FORMAT_TOML: return Properties::Format::TOML;
        case SANDBOX_FORMAT_YAML: return Properties::Format::YAML;
        default: return Properties::Format::JSON;
    }
}

extern "C" {

    sandbox_properties_t* sandbox_properties_create(void) {
        return reinterpret_cast<sandbox_properties_t*>(new Properties());
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

    void sandbox_properties_clear(sandbox_properties_t* props, const char** path, size_t path_len) {
        if (props) cast(props)->clear(make_path(path, path_len));
    }

    bool sandbox_properties_has(const sandbox_properties_t* props, const char** path, size_t path_len) {
        if (!props) return false;
        return cast(props)->has(make_path(path, path_len));
    }

    char** sandbox_properties_keys(const sandbox_properties_t* props, const char** path, size_t path_len, size_t* out_count) {
        if (!props || !out_count) return nullptr;

        auto cpp_keys = cast(props)->keys(make_path(path, path_len));
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

    void sandbox_properties_merge(sandbox_properties_t* props, const char** path, size_t path_len, const sandbox_properties_t* other) {
        if (props && other) {
            cast(props)->merge(make_path(path, path_len), *cast(other));
        }
    }

    sandbox_properties_t* sandbox_properties_sub(const sandbox_properties_t* props, const char** path, size_t path_len) {
        if (!props) return nullptr;
        Properties sub_props = cast(props)->sub(make_path(path, path_len));
        return reinterpret_cast<sandbox_properties_t*>(new Properties(std::move(sub_props)));
    }

    // --- GETTERS ---

    bool sandbox_properties_get_int64(const sandbox_properties_t* props, const char** path, size_t path_len, int64_t* out_val) {
        if (!props || !out_val) return false;
        if (auto val = cast(props)->get<int64_t>(make_path(path, path_len))) {
            *out_val = *val;
            return true;
        }
        return false;
    }

    bool sandbox_properties_get_double(const sandbox_properties_t* props, const char** path, size_t path_len, double* out_val) {
        if (!props || !out_val) return false;
        if (auto val = cast(props)->get<double>(make_path(path, path_len))) {
            *out_val = *val;
            return true;
        }
        return false;
    }

    bool sandbox_properties_get_bool(const sandbox_properties_t* props, const char** path, size_t path_len, bool* out_val) {
        if (!props || !out_val) return false;
        if (auto val = cast(props)->get<bool>(make_path(path, path_len))) {
            *out_val = *val;
            return true;
        }
        return false;
    }

    const char* sandbox_properties_get_string(const sandbox_properties_t* props, const char** path, size_t path_len) {
        if (!props) return nullptr;
        if (auto val = cast(props)->get<std::string>(make_path(path, path_len))) {
            // Use thread_local to safely pass string pointer across ABI without requiring the caller to free it
            thread_local std::string scratchpad;
            scratchpad = std::move(*val);
            return scratchpad.c_str();
        }

        return nullptr;
    }

    // --- SETTERS ---

    void sandbox_properties_set_int64(sandbox_properties_t* props, const char** path, size_t path_len, int64_t val) {
        if (props) cast(props)->set<int64_t>(make_path(path, path_len), val);
    }

    void sandbox_properties_set_double(sandbox_properties_t* props, const char** path, size_t path_len, double val) {
        if (props) cast(props)->set<double>(make_path(path, path_len), val);
    }

    void sandbox_properties_set_bool(sandbox_properties_t* props, const char** path, size_t path_len, bool val) {
        if (props) cast(props)->set<bool>(make_path(path, path_len), val);
    }

    void sandbox_properties_set_string(sandbox_properties_t* props, const char** path, size_t path_len, const char* val) {
        if (props && val) {
            std::string cpp_val(val);
            cast(props)->set<std::string>(make_path(path, path_len), cpp_val);
        }
    }
}