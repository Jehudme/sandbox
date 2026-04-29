#pragma once
#include "ecs.h"

namespace sandbox
{
    class scope_guard
    {
    public:
        scope_guard(entity entt);
        ~scope_guard();

    private:
        world m_ecs;
    };
}

/**
 * These helper macros allow the __LINE__ macro to expand into an actual
 * number before the token pasting (##) happens.
 */
#define SANBOX_CONCAT_IMPL(x, y) x##y
#define SANBOX_CONCAT(x, y) SANBOX_CONCAT_IMPL(x, y)

/**
 * Creates a unique scope_guard variable for the current line.
 * Usage: SANDBOX_SCOPE_GUARD(my_plugin_entity);
 */
#define SANDBOX_SCOPE_GUARD(entt) \
sandbox::scope_guard SANBOX_CONCAT(scope_guard_, __LINE__)(entt)