#pragma once

#include <rttr/registration>

// These macros must be used *inside a single* RTTR_REGISTRATION { ... } block.

#define SANDBOX_REGISTRATION \
RTTR_REGISTRATION

#define SANDBOX_REGISTER_CLASS(class_type, identifier) \
rttr::registration::class_<class_type>(identifier) \
.constructor<>()(rttr::policy::ctor::as_raw_ptr);

#define SANDBOX_REGISTER_CLASS_DEFAULT(class_type) \
SANDBOX_REGISTER_CLASS(class_type, #class_type)

#define SANDBOX_REGISTER_GLOBAL_FUNCTION(function_ptr, identifier) \
rttr::registration::method(identifier, function_ptr);

#define SANDBOX_REGISTER_STATIC_FUNCTION(class_type, function_ptr, identifier) \
rttr::registration::class_<class_type>(#class_type) \
.method(identifier, function_ptr);