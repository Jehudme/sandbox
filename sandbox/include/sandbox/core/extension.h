#pragma once

#include "sandbox/utilities/properties.h"
#include "sandbox/utilities/scoped_logger.h"
#include <memory>

namespace sandbox
{
    class extension
    {
    public:
        explicit extension() = default;
        virtual ~extension() = default;

        void initialize(const properties& extension_properties);

        void finalize();

        logger& get_logger() const;

    private:
        virtual void on_initialize(const properties& extension_properties) = 0;
        virtual void on_finalize() = 0;

        std::unique_ptr<logger> _logger;
    };
}