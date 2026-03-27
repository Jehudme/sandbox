#include <format>

namespace sandbox
{
    template <typename ... argument_types>
    void logger::log(level lvl, std::string_view format_string, argument_types&&... arguments) const
    {
        _internal_log(lvl, std::vformat(format_string, std::make_format_args(arguments...)));
    }

    template<typename... argument_types>
    void logger::trace(std::string_view format_string, argument_types&&... arguments) const
    {
        _internal_log(level::trace, std::vformat(format_string, std::make_format_args(arguments...)));
    }

    template<typename... argument_types>
    void logger::debug(std::string_view format_string, argument_types&&... arguments) const
    {
        _internal_log(level::debug, std::vformat(format_string, std::make_format_args(arguments...)));
    }

    template<typename... argument_types>
    void logger::info(std::string_view format_string, argument_types&&... arguments) const
    {
        _internal_log(level::info, std::vformat(format_string, std::make_format_args(arguments...)));
    }

    template<typename... argument_types>
    void logger::warn(std::string_view format_string, argument_types&&... arguments) const
    {
        _internal_log(level::warn, std::vformat(format_string, std::make_format_args(arguments...)));
    }

    template<typename... argument_types>
    void logger::error(std::string_view format_string, argument_types&&... arguments) const
    {
        _internal_log(level::error, std::vformat(format_string, std::make_format_args(arguments...)));
    }

    template<typename... argument_types>
    void logger::critical(std::string_view format_string, argument_types&&... arguments) const
    {
        _internal_log(level::critical, std::vformat(format_string, std::make_format_args(arguments...)));
    }
}