#pragma once
#include "sandbox//utilities/properties.h"

namespace sandbox
{
    class engine;

    class extension
    {
        friend class engine;
    public:
        extension() = default;
        virtual ~extension() = default;

        virtual void initialize(const properties& properties) = 0;
        virtual void finalize() = 0;

        engine& get_engine() const;

    protected:
        engine* m_engine = nullptr;

    };
}
