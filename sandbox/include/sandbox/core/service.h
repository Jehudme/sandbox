#pragma once
#include <cstdint>

namespace sandbox {

    #define SANDBOX_DECLARE_SERVICE(service_name, interface_type) \
        struct service_name { \
            static constexpr const char* type_name = #service_name; \
            interface_type* api{nullptr}; \
            uint8_t version_major{1}; \
            uint8_t version_minor{0}; \
        };

    // The dummy lock also gets a name so the templates don't break
    struct no_lock {
        static constexpr const char* type_name = "no_lock";
    };

}