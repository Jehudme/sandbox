#include "../../include/sandbox/utilities/scoped_logger.h"

namespace sandbox
{
    thread_local std::vector<std::reference_wrapper<logger>> scoped_logger::_logger_stack;

    scoped_logger::scoped_logger(logger& logger_instance)
    {
        _logger_stack.push_back(std::ref(logger_instance));
    }

    scoped_logger::~scoped_logger()
    {
        if (!_logger_stack.empty())
        {
            _logger_stack.pop_back();
        }
    }

    logger& scoped_logger::get_current_logger()
    {
        if (_logger_stack.empty())
        {
            return _get_default_logger();
        }

        return _logger_stack.back().get();
    }

    logger& scoped_logger::_get_default_logger()
    {
        static logger default_system_logger("sandbox", logger::level::info);
        return default_system_logger;
    }
}