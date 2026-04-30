#include "sandbox/core/plugin.h"
#include "sandbox/core/engine.h"

namespace sandbox
{
    plugin::plugin(engine& context) :
        context(context)
    {
    }

    plugin::plugin(engine* context) :
        plugin(*context)
    {
    }

    plugin::~plugin() = default;
}
