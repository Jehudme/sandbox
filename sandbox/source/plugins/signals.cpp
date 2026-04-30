#include "sandbox/plugins/signals.h"

#include "sandbox/core/type_registration.h"

SANDBOX_REFLECTION {
    SANDBOX_REGISTER_TYPE_NAMED(sandbox::signals, "default::signals");
}

namespace sandbox
{
    signals::signals(engine& context) : plugin(context)
    {
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