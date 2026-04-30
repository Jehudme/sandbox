#pragma once

#include <any>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "sandbox/core/plugin.h"

namespace sandbox
{
    /**
     * dispatcher — core event-bus plugin.
     *
     * Provides a type-safe publish/subscribe mechanism backed by std::type_index.
     * Publish transfers ownership of the event to the dispatcher; all registered
     * handlers receive a const-reference and the event is destroyed afterwards.
     *
     * Register with the engine manifest as:
     *   { "plugins": { "<alias>": { "type": "sandbox::dispatcher" } } }
     */
    class dispatcher : public plugin
    {
    public:
        explicit dispatcher(engine* context);
        ~dispatcher() override;

        template<typename event_type>
        void publish(std::unique_ptr<event_type> event);

        template<typename event_type>
        void subscribe(std::function<void(const event_type&)> callback);

    private:
        void initialize() override;
        void finalize()   override;

        std::unordered_map<std::type_index, std::vector<std::any>> m_handlers;
    };
}

#include "sandbox/core/dispatcher.inl"
