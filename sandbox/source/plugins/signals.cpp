#include "sandbox/plugins/signals.h"
#include "sandbox/core/type_registration.h"

SANDBOX_REFLECTION
{
    SANDBOX_REGISTER_PLUGIN_NAMED(sandbox::signals, "default::signals")
}

namespace sandbox
{
    signals::signals(engine& context) : plugin(context)
    {
        // 5. Create the dedicated router entity and attach the tag
        m_bus_entity = context.ecs.entity("::internal::global_event_bus");
        m_bus_entity.add<global_event_bus>();
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
}