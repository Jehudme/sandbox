#include "../../include/sandbox/core/engine.h"

namespace sandbox
{
    engine::engine(const properties& manifest) {
        initialize(manifest);
    }

    engine::~engine() {
        finalize();
    }

    void engine::initialize(const properties& manifest)
    {
    }

    void engine::finalize()
    {
    }
}
