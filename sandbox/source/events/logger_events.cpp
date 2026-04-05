#include "sandbox/events/logger_events.h"

namespace sandbox::events
{
    log_event::log_event(plugins::logger_plugin::log_level level, std::string formatted_message) :
    level(level), message(formatted_message) {}
}
