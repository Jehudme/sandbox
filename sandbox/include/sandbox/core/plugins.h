#pragma once
#include "sandbox//utils/properties.h"

namespace sandbox
{
    class engine;

    class plugins
    {
        friend class engine;
    public:
        plugins() = default;
        virtual ~plugins() = default;

        virtual void initialize(const properties& properties) = 0;
        virtual void finalize() = 0;

        engine& get_engine() const;

    protected:
        engine* m_engine = nullptr;

    };
}
