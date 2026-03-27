#include "sandbox/core/extension.h"

namespace sandbox
{
    void extension::initialize(const properties& extension_properties)
    {
        // 1. Extract logger configuration from the properties tree
        const properties logger_properties = extension_properties.sub_properties({"logger"});

        const std::string logger_name = logger_properties
            .get<std::string>({"name"})
            .value_or("unnamed");

        const std::string logger_level_string = logger_properties
            .get<std::string>({"level"})
            .value_or("info");

        const bool is_logger_async = logger_properties
            .get<bool>({"async"})
            .value_or(false);


    }

    void extension::finalize()
    {

    }
}
