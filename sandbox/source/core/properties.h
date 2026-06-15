#pragma once
#include <string>
#include <glaze/glaze.hpp>

namespace sandbox::core {
    class properties_t {
    public:
        using path_t = std::vector<std::string>;
        using keys_t = std::vector<std::string>;

        enum Format { JSON, BEVE, TOML, YAML };

        properties_t();
        ~properties_t();

        void load(std::string_view data, Format format);
        std::string dump(Format format) const;

        void clear(const path_t& path);

        bool has(const path_t& path) const;
        keys_t keys(const path_t& path) const;

        void merge(const path_t& path, const properties_t& other);

        properties_t sub(const path_t& path) const;

        template <typename Type>
        std::optional<Type> get(const path_t& path = {}) const;
        template <typename Type>
        void set(const path_t& path, Type value);

    private:
        glz::generic m_data;
    };
}

#include "detail/properties.inl"