#pragma once

#include "sandbox/core/extension.h"
#include <string_view>

namespace sandbox::extensions
{
    // Tag applied to transient event source entities so the cleanup system can find and
    // destroy them at end-of-frame (PostUpdate).
    struct transient_event_tag {};

    class events : public sandbox::extension
    {
    public:
        void initialize(const sandbox::properties& properties) override;
        void finalize() override;

        // Model A (event-based): creates a transient source entity, attaches the payload,
        // and emits the event immediately via world.event<>().emit().
        // Observers (subscriptions) are called synchronously before publish() returns.
        // The source entity is cleaned up at the end of the frame (PostUpdate).
        template<typename event_type>
        void publish(event_type&& event_data);

        // Subscribes to events of type event_type with a named observer stored at
        // ::events::<name>, so destroy/enable/disable can reliably find it.
        template<typename event_type>
        void subscribe(std::string_view name, auto&& callback);

        // Manage named subscriptions in the ::events:: namespace only.
        void destroy(std::string_view name);
        void enable(std::string_view name);
        void disable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;
    };
}

#include "events.inl"