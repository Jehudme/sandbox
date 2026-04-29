#include "sandbox/core/scope_guard.h"
#include <stack>

namespace sandbox
{
    scope_guard::scope_guard(flecs::entity entt) : m_ecs(entt.world())
    {
        m_ecs.set_scope(entt);

        m_ecs.entity("::internal::scopes")
             .ensure<std::stack<flecs::entity>>()
             .push(entt);
    }

    scope_guard::~scope_guard()
    {
        auto& scopes = m_ecs.entity("::internal::scopes").ensure<std::stack<flecs::entity>>();

        if (!scopes.empty()) {
            scopes.pop();
        }

        if (!scopes.empty()) {
            m_ecs.set_scope(scopes.top());
        } else {
            m_ecs.set_scope(0);
        }
    }
}