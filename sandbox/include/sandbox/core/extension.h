#pragma once

#include "sandbox/filesystem/properties.h"
#include <memory>

namespace sandbox
{
    class engine; // Forward declaration

    /**
     * @brief Base class for all engine modules.
     * Enforces a unique identity by disabling all copy and move operations.
     */
    class extension
    {
    public:
        explicit extension() = default;
        virtual ~extension() = default;

        // --- Non-Copyable & Non-Movable ---
        extension(const extension&) = delete;
        extension& operator=(const extension&) = delete;
        extension(extension&&) = delete;
        extension& operator=(extension&&) = delete;

        virtual void initialize(engine& app, const properties& props) = 0;
        virtual void finalize(engine& app) = 0;
    };
}