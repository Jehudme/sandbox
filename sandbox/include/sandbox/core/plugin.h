#pragma once
#include "sandbox//utils/properties.h"

namespace sandbox
{
    class engine;

    class plugin
    {
        friend class engine;
    public:
        plugin() = default;
        virtual ~plugin() = default;

        virtual void initialize(const properties& properties) = 0;
        virtual void finalize() = 0;

        engine& get_engine() const;

    protected:
        engine* m_engine = nullptr;

    };
}
