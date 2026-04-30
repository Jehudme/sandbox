#include "sandbox/core/dispatcher.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/type_registration.h"

namespace sandbox
{
    dispatcher::dispatcher(engine* context)
        : plugin(context)
    {
    }

    dispatcher::~dispatcher() = default;

    void dispatcher::initialize()
    {
        // Nothing to acquire; subscribers register after the engine is initialised.
    }

    void dispatcher::finalize()
    {
        m_handlers.clear();
    }
}

SANDBOX_REFLECTION
{
    SANDBOX_REGISTER_PLUGIN(sandbox::dispatcher)
}
