#include "sandbox/extensions/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void logger::initialize(sandbox::engine& app, const sandbox::properties& props)
    {
        const std::string logger_name = props.get<std::string>({"logger", "name"}).value_or("extension_logger");
        const std::string logger_level_string = props.get<std::string>({"logger", "level"}).value_or("INFO");
        const bool is_logger_async = props.get<bool>({"logger", "async"}).value_or(false);

        const sandbox::logger::level logger_level = sandbox::logger::string_to_level(logger_level_string);

        _logger = std::make_unique<sandbox::logger>(logger_name, logger_level, is_logger_async);

        app._log(sandbox::logger::level::info, "extensions::logger: initialized name='{}' level='{}'",
                 logger_name, sandbox::logger::level_to_string(logger_level));
    }

    void logger::finalize(sandbox::engine& app)
    {
        app._log(sandbox::logger::level::info, "extensions::logger: finalized");
        _logger.reset();
    }
}
