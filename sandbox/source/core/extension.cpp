#include "sandbox/core/extension.h"

namespace sandbox
{
    engine& extension::get_engine() const
    {
        return *m_engine_instance;
    }
}
