#include "sandbox/core/plugins.h"

namespace sandbox
{
    engine& plugins::get_engine() const
    {
        return *m_engine;
    }
}
