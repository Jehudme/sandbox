#include "sandbox/plugins/dispatcher.h"

#include "sandbox/core/type_registration.h"

SANDBOX_REFLECTION {
    SANDBOX_REGISTER_TYPE_NAMED(sandbox::dispatcher, "sandbox::dispatcher");
}

namespace sandbox
{
    dispatcher::dispatcher(engine& context) : plugin(context)
    {
    }

    void dispatcher::initialize()
    {
    }

    void dispatcher::finalize()
    {
    }
}