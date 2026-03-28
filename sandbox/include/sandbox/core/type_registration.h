#pragma once

#include <rttr/registration>

#define SANDBOX_REFLECTION \
RTTR_REGISTRATION

#define SANDBOX_REGISTER_TYPE_NAMED(type_t, metadata_name) \
rttr::registration::class_<type_t>(metadata_name) \
.constructor<>()(rttr::policy::ctor::as_raw_ptr);

#define SANDBOX_REGISTER_TYPE(type_t) \
SANDBOX_REGISTER_TYPE_NAMED(type_t, #type_t)

#define SANDBOX_REGISTER_GLOBAL_FUNCTION(function_address, metadata_name) \
rttr::registration::method(metadata_name, function_address);

#define SANDBOX_REGISTER_STATIC_METHOD(type_t, function_address, metadata_name) \
rttr::registration::class_<type_t>(#type_t) \
.method(metadata_name, function_address);
