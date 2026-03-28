#pragma once

#include "properties.h"
#include <memory>

namespace sandbox
{
    class engine;

    class extension
    {
        friend class engine;

    public:
        explicit extension() = default;
        virtual ~extension() = default;

        extension(const extension&) = delete;
        extension& operator=(const extension&) = delete;
        extension(extension&&) = delete;
        extension& operator=(extension&&) = delete;

        virtual void initialize(const properties& props) = 0;
        virtual void finalize() = 0;

    protected:
        engine* _app = nullptr;
    };
}
