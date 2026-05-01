#pragma once
#include "ecs.h"


namespace sandbox{

    class plugin {
        friend class engine;

    public:
        plugin(world ecs);
        ~plugin();

        virtual void initialize() = 0;
        virtual void finalize() = 0;

    protected:
        world ecs;
    };
}
