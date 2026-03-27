#pragma once

#include "sandbox/core/extension.h"
#include <string_view>

namespace sandbox::extensions
{
    struct transient_event_tag {};

    class events : public sandbox::extension
    {
    public:
        void initialize(const sandbox::properties& properties) override;
        void finalize() override;

        template<typename event_type>
        void publish(event_type event_data);

        template<typename event_type>
        void subscribe(std::string_view name, auto&& callback);

        template<typename event_type>
        void subscribe(std::string_view name, std::string_view stage, auto&& callback);

        void destroy(std::string_view name);
        void enable(std::string_view name);
        void disable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;
    };
}

#include "events.inl"