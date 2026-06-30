#pragma once
#include <string>
#include <glaze/glaze.hpp>

namespace sandbox::core {
    /**
     * @brief A generic dynamic properties container based on glaze.
     */
    class properties_t {
    public:
        using path_t = std::vector<std::string>;
        using keys_t = std::vector<std::string>;

        /**
         * @brief Supported serialization formats.
         */
        enum Format { JSON, BEVE, TOML, YAML };

        /**
         * @brief Constructs a new empty properties object.
         */
        properties_t();

        /**
         * @brief Destroys the properties object.
         */
        ~properties_t();

        /**
         * @brief Loads properties from a string buffer.
         * @param data The string data to parse.
         * @param format The data format.
         */
        void load(std::string_view data, Format format);

        /**
         * @brief Dumps properties into a string.
         * @param format The target data format.
         * @return The serialized string representation.
         */
        std::string dump(Format format) const;

        /**
         * @brief Clears the property at the given path.
         * @param path The key path.
         */
        void clear(const path_t& path);

        /**
         * @brief Checks if a property exists at the given path.
         * @param path The key path.
         * @return true if it exists, false otherwise.
         */
        bool has(const path_t& path) const;

        /**
         * @brief Retrieves all sub-keys at the given path.
         * @param path The key path.
         * @return A list of string keys.
         */
        keys_t keys(const path_t& path) const;

        /**
         * @brief Merges another properties object into the specified path.
         * @param path The target key path.
         * @param other The properties object to merge.
         */
        void merge(const path_t& path, const properties_t& other);

        /**
         * @brief Returns a copy of the properties located at the specified path.
         * @param path The key path.
         * @return A new properties object containing the sub-tree.
         */
        properties_t sub(const path_t& path) const;

        /**
         * @brief Retrieves the strongly-typed value at the given path.
         * @tparam Type The expected value type.
         * @param path The key path.
         * @return An optional containing the value if found, or nullopt.
         */
        template <typename Type>
        std::optional<Type> get(const path_t& path = {}) const;

        /**
         * @brief Sets a strongly-typed value at the given path.
         * @tparam Type The value type.
         * @param path The key path.
         * @param value The value to set.
         */
        template <typename Type>
        void set(const path_t& path, Type value);

    private:
        glz::generic m_data;
    };
}

#include "detail/properties.inl"