#pragma once
#include "sandbox//utils/properties.h"

namespace sandbox
{
    class engine;

    class plugin
    {
        friend class engine;
    public:
        plugin(engine& context);
        virtual ~plugin();

        engine& context;

    private:
        virtual void initialize() = 0;
        virtual void finalize() = 0;

    };
}
