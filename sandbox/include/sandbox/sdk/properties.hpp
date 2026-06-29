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

        explicit properties(sandbox_handle_t raw);

        bool load(const std::string& data, Format format = Format::JSON);
        std::string dump(Format format = Format::JSON) const;

        void clear(const std::string& path);
        bool has(const std::string& path) const;
        std::vector<std::string> keys(const std::string& path) const;
        
        void merge(const std::string& path, const properties& other);
        properties sub(const std::string& path) const;

        template <typename T>
        bool get(const std::string& path, T& out_val) const;

        template <typename T>
        std::optional<T> get(const std::string& path) const;

        template <typename T>
        bool get_array(const std::string& path, std::vector<T>& out_val) const;

        template <typename T>
        void set(const std::string& path, const T& value);

        template <typename T>
        void set_array(const std::string& path, const std::vector<T>& values);

        sandbox_handle_t get_raw() const { return m_handle; }
        
    private:
        sandbox_handle_t m_handle;
    };
}

#include "detail/properties.inl"
