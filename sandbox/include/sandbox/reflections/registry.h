#pragma once

#include <memory>
#include <string>

namespace sandbox
{
    /**
     * @brief Static registry for creating engine objects by string identifier.
     * This class uses RTTR internally but hides all reflection dependencies from the user.
     */
    class registry
    {
    public:
        registry() = delete;
        ~registry() = delete;

        /**
         * @brief Creates a heap-allocated instance of a registered type and wraps it in a unique_ptr.
         * @tparam base_type The expected base class (e.g., component, system).
         * @param type_identifier The string name registered in RTTR.
         * @return std::unique_ptr<base_type> A managed pointer to the new instance, or nullptr if failed.
         */
        template<typename base_type>
        static std::unique_ptr<base_type> create_type(const std::string& type_identifier);

        static bool is_type_registered(const std::string& type_identifier);

        /**
         * @brief Returns the official registered name of a type identifier.
         */
        static std::string get_registered_name(const std::string& type_identifier);

    private:
        // This is the "bridge" function that allows us to hide RTTR in the .cpp file.
        static void* _internal_create_instance(const std::string& type_identifier);
    };
}

#include "../core/registry.inl"
