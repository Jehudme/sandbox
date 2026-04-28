#include "sandbox/core/plugin.h"

namespace sandbox
{
    engine& plugin::get_engine() const
    {
        return m_engine;
    }
}
