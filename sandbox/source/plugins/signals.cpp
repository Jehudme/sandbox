#include "sandbox/plugins/signals.h"
#include "sandbox/core/type_registration.h"

SANDBOX_REFLECTION
{
    SANDBOX_REGISTER_PLUGIN(sandbox::signals)
}

namespace sandbox
{
    signals::signals(engine& context) : plugin(context)
    {
        // Initialize the default fallback bus when the plugin is created
        m_default_bus = context.ecs.entity("::internal::default_event_bus");
        m_default_bus.add<signal_channel>();
    }

    signals::~signals()
    {
    }

    void signals::initialize()
    {
    }

    void signals::finalize()
    {
    }

    flecs::entity signals::create_channel(std::string_view name)
    {
        // Creates a custom isolated bus entity and flags it as a valid channel
        return context.ecs.entity(name.data()).add<signal_channel>();
    }
}