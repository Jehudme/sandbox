#pragma once
#include "sandbox//utils/properties.h"

namespace sandbox
{
    class engine;

    class plugin
    {
        friend class engine;
    public:
        plugin(engine& engine_instance);
        virtual ~plugin();

        engine& get_engine() const;

    protected:
        engine& m_engine;

    private:
        virtual void initialize() = 0;
        virtual void finalize() = 0;

    };
}
