#pragma once

#include <rttr/registration>

/**
 * @brief Registers a class with a custom string identifier.
 * Use this in your .cpp file.
 * * @param class_type The C++ type of the class (e.g., sandbox::transform_component).
 * @param identifier The string name used to create it (e.g., "transform").
 */
#define SANDBOX_REGISTER_CLASS(class_type, identifier) \
RTTR_REGISTRATION \
{ \
    rttr::registration::class_<class_type>(identifier) \
        .constructor<>()(rttr::policy::ctor::as_raw_ptr); \
}

/**
 * @brief Registers a class using its literal type name as the identifier.
 * Use this in your .cpp file.
 * * @param class_type The C++ type to register.
 */
#define SANDBOX_REGISTER_CLASS_DEFAULT(class_type) \
    SANDBOX_REGISTER_CLASS(class_type, #class_type)