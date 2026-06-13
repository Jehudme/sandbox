#pragma once
#include <string>
#include <glaze/glaze.hpp>

namespace sandbox::core {
    class Properties {
    public:
        using Path = std::vector<std::string>;
        using Keys = std::vector<std::string>;

        enum Format { JSON, BEVE, TOML, YAML };

        Properties();
        ~Properties();

        void load(std::string_view data, Format format);
        std::string dump(Format format) const;

        void clear(const Path& path);

        bool has(const Path& path) const;
        Keys keys(const Path& path) const;

        void merge(const Path& path, const Properties& other);

        Properties sub(const Path& path) const;

        template <typename Type>
        std::optional<Type> get(const Path& path = {}) const;
        template <typename Type>
        void set(const Path& path, Type& value);

    private:
        glz::generic m_data;
    };
}

#include "detail/properties.inl"