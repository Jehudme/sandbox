#pragma once
#include "sandbox/core/engine.h"

namespace sandbox {
    class logger : public plugin
    {
    public:
        logger(engine& context);
        ~logger();

        void initialize() override;
        void finalize() override;

    };
}
