#include "sandbox/plugins/logger_plugin.h"

namespace sandbox::plugins
{
    void logger_plugin::initialize(const properties& properties)
    {
    }

    void logger_plugin::finalize()
    {
    }

    void logger_plugin::_internal_log(log_level log_level, std::string_view formatted_message) const
    {
    }

    std::string level_to_string(logger_plugin::log_level log_level)
    {
    }

    logger_plugin::log_level string_to_level(std::string_view log_level_string)
    {
    }
}
