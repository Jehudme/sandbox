#pragma once

#include "sandbox/core/extension.h"

namespace sanbox::extensions
{
    class events : public sandbox::extension
    {
    public:
        template<typename event_t>
        using logic_lambda = std::function<void(sandbox::engine&, event_t&)>;

        template<typename event_t>
        void publish(event_t&& event_data);

        template<typename event_t>
        void subscribe(std::string_view name, logic_lambda<event_t>&& callback); // will run instantanly

        template<typename event_t>
        void subscribe(std::string_view name, std::string_view stage, logic_lambda<event_t>&& callback); // will run in the specified stage (or default stage if empty)

        void destroy(std::string_view name);
        void disable(std::string_view name);
        void enable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;

    };
}