#pragma once

#include <vector>
#include <functional>
#include "logger.h"

namespace sandbox
{
    class scoped_logger
    {
    public:
        explicit scoped_logger(logger& logger_instance);
        ~scoped_logger();

        scoped_logger(const scoped_logger&) = delete;
        scoped_logger& operator=(const scoped_logger&) = delete;
        scoped_logger(scoped_logger&& other_logger) noexcept = default;
        scoped_logger& operator=(scoped_logger&& other_logger) noexcept = default;

        static logger& get_current_logger();

    private:
        static thread_local std::vector<std::reference_wrapper<logger>> _logger_stack;

        static logger& _get_default_logger();
    };
}


#include "scoped_logger.inl"
