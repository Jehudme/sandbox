#pragma once
#include "sandbox/abi/properties.h"
#include <string>
#include <vector>
#include <optional>

namespace sandbox {
    class properties {
    public:
        enum class Format { JSON, BEVE, TOML, YAML };

        properties();
        ~properties();

        properties(const properties&) = delete;
        properties& operator=(const properties&) = delete;

        properties(properties&& other) noexcept;
        properties& operator=(properties&& other) noexcept;

        explicit properties(sandbox_properties_t* raw);

        bool load(const std::string& data, Format format = Format::JSON);
        std::string dump(Format format = Format::JSON) const;

        void clear(const std::string& path);
        bool has(const std::string& path) const;
        std::vector<std::string> keys(const std::string& path) const;
        
        void merge(const std::string& path, const properties& other);
        properties sub(const std::string& path) const;

        bool get_int64(const std::string& path, int64_t& out_val) const;
        bool get_double(const std::string& path, double& out_val) const;
        bool get_bool(const std::string& path, bool& out_val) const;
        bool get_string(const std::string& path, std::string& out_val) const;
        bool get_string_array(const std::string& path, std::vector<std::string>& out_val) const;

        void set_int64(const std::string& path, int64_t val);
        void set_double(const std::string& path, double val);
        void set_bool(const std::string& path, bool val);
        void set_string(const std::string& path, const std::string& val);
        void set_string_array(const std::string& path, const std::vector<std::string>& values);

        template <typename Type>
        std::optional<Type> get(const std::string& path) const;

        template <typename Type>
        void set(const std::string& path, const Type& value);

        sandbox_properties_t* get_raw() const { return m_props; }
        
    private:
        sandbox_properties_t* m_props;
    };
}

#include "detail/properties.inl"
