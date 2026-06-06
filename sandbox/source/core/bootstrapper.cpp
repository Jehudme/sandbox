#include "sandbox/core/bootstrapper.h"
#include "sandbox/event_bus/logger_events.h"

#include <algorithm>
#include <format>
#include <stdexcept>

// File-local helper: emit an ERROR log (if the logger service is live) then
// unconditionally throw std::runtime_error.  SANDBOX_ERROR_THROW relies on the
// logger's own throw path which is a no-op when no logger is registered (e.g.
// in unit tests).  This macro guarantees the throw regardless.
#define BOOT_FATAL(world, fmt_str, ...) \
    do { \
        std::string _boot_msg = std::format(fmt_str, ##__VA_ARGS__); \
        SANDBOX_ERROR(world, "{}", _boot_msg); \
        throw std::runtime_error(_boot_msg); \
    } while(0)


namespace sandbox {

    // =========================================================================
    // Construction
    // =========================================================================

    bootstrapper::bootstrapper(flecs::world& ecs) {
        ecs.module<sandbox::bootstrapper>("::Bootstrapper");
        // No log here; the logger subsystem may not yet be active at this point.
    }

    // =========================================================================
    // Public API
    // =========================================================================

    void bootstrapper::stage(const std::vector<module_info>& info) {
        // Append every descriptor verbatim — intentional; multi-version coexistence
        // is legal at the staging phase. Deduplication happens in resolve_activations().
        m_modules.insert(m_modules.end(), info.begin(), info.end());
    }

    void bootstrapper::activate(const std::string& module_name) {
        // Verify at least one staged variant exists before enqueuing.
        for (const auto& mod : m_modules) {
            if (mod.name == module_name) {
                m_explicit_activations.push_back(module_name);
                return;
            }
        }

        // Issue 5 / Clean Failures: a manifest-requested name that exists in no
        // staged library is a fatal configuration error — fail loudly now rather
        // than silently entering an empty main loop.
        throw std::runtime_error(
            "[Bootstrapper] activate() failed: module '" + module_name +
            "' was not found in any staged library. "
            "Ensure the library is loaded before calling execute().");
    }

    void bootstrapper::execute(flecs::world& ecs) {
        // --- Phase 1: cascade hard dependencies, resolve versions, detect conflicts.
        resolve_activations(ecs);

        // --- Phase 2: detect and defuse manifest service collisions.
        audit_service_collisions(ecs);

        // --- Phase 3: multi-pass topological import — only strictness::require
        //              edges are treated as ordering constraints (Issue 5).
        bool processing = true;
        while (processing) {
            bool progressed_this_cycle = false;

            for (auto& [mod_name, mod_idx] : m_active_modules) {
                module_info& mod = m_modules[mod_idx];
                if (mod.is_loaded) continue;

                bool ready_to_boot = true;
                for (const auto& req : mod.requirements) {
                    // strictness::expect must NOT block the topo loop (Issue 5).
                    if (req.policy != requirement::strictness::require) continue;

                    if (req.target_kind == requirement::kind::module) {
                        if (!is_module_loaded(req.target_name)) {
                            ready_to_boot = false;
                            break;
                        }
                    } else if (req.target_kind == requirement::kind::service) {
                        if (!is_service_loaded(req.target_name)) {
                            ready_to_boot = false;
                            break;
                        }
                    }
                }

                if (ready_to_boot) {
                    if (mod.import_fn) mod.import_fn(ecs);
                    mod.is_loaded        = true;
                    progressed_this_cycle = true;
                    SANDBOX_INFO(ecs, "[Bootstrapper] Loaded module: {} v{}.{}",
                                 mod.name, mod.version_major, mod.version_minor);
                }
            }

            if (!progressed_this_cycle) {
                if (all_activated_modules_loaded()) {
                    break;
                }

                // No module made progress and the active set is not fully loaded:
                // this is an unresolvable dependency cycle.
                BOOT_FATAL(ecs,
                    "[Bootstrapper] Dependency deadlock detected: circular or "
                    "unresolvable hard requirement in the activated module graph.");
            }
        }

        SANDBOX_INFO(ecs, "[Bootstrapper] All activated modules loaded successfully.");
    }

    // =========================================================================
    // Private — Phase 1: Graph Resolution
    // =========================================================================

    void bootstrapper::resolve_activations(flecs::world& ecs) {
        m_active_modules.clear();
        m_version_constraints.clear();

        // Seed the work-list with explicitly requested names (from manifest).
        // For explicit activations we have no version constraint, so we pick the
        // highest available version automatically.
        for (const auto& name : m_explicit_activations) {
            if (m_active_modules.count(name)) continue; // already selected

            const std::size_t npos = m_modules.size();
            std::size_t best       = find_best_module_version(name, 0, 0);
            // find_best_module_version with (0, 0) selects the highest version overall.
            if (best == npos) {
                // activate() already guards against this, but be defensive.
                BOOT_FATAL(ecs,
                    "[Bootstrapper] Explicit activation '{}' has no staged variant "
                    "(internal inconsistency — was stage() called before execute()?).", name);
            }
            m_active_modules[name] = best;
        }

        // Cascade hard (require) dependencies until the active set stabilises.
        bool graph_changed = true;
        while (graph_changed) {
            graph_changed = false;

            // Iterate over a snapshot of current keys to avoid invalidating the
            // map while potentially inserting new entries.
            std::vector<std::string> current_names;
            current_names.reserve(m_active_modules.size());
            for (const auto& [k, _] : m_active_modules) current_names.push_back(k);

            for (const auto& active_name : current_names) {
                const module_info& mod = m_modules[m_active_modules.at(active_name)];

                for (const auto& req : mod.requirements) {
                    // Issue 5: only hard dependencies drive cascade.
                    if (req.policy != requirement::strictness::require) continue;

                    if (req.target_kind == requirement::kind::module) {
                        // ----- Module dependency --------------------------------
                        auto it = m_active_modules.find(req.target_name);
                        if (it == m_active_modules.end()) {
                            // Module not yet in active set — select best version.
                            std::size_t best = find_best_module_version(
                                req.target_name, req.min_major, req.min_minor);

                            if (best == m_modules.size()) {
                                BOOT_FATAL(ecs,
                                    "[Bootstrapper] Unsatisfied hard module dependency: "
                                    "'{}' (required by '{}') has no staged variant "
                                    "matching v{}.{}+.",
                                    req.target_name, mod.name,
                                    req.min_major, req.min_minor);
                            }

                            m_active_modules[req.target_name] = best;
                            m_version_constraints[req.target_name] = {
                                req.min_major, req.min_minor, mod.name
                            };
                            graph_changed = true;

                        } else {
                            // Module already selected — verify version compatibility
                            // and detect Issue 1 (conflicting version demands).
                            const module_info& chosen = m_modules[it->second];

                            // Check if the already-chosen variant satisfies this requirement.
                            const bool major_ok = (chosen.version_major == req.min_major);
                            const bool minor_ok = (chosen.version_minor >= req.min_minor);
                            const bool zero_req = (req.min_major == 0);  // unconstrained

                            if (!zero_req && !(major_ok && minor_ok)) {
                                // The active variant doesn't satisfy this edge's constraint.
                                // Check whether there was a prior constraint with a different
                                // major to detect a hard version conflict (Issue 1).
                                auto constraint_it = m_version_constraints.find(req.target_name);
                                if (constraint_it != m_version_constraints.end() &&
                                    constraint_it->second.min_major != req.min_major)
                                {
                                    BOOT_FATAL(ecs,
                                        "[Bootstrapper] Fatal version conflict: '{}' requires "
                                        "'{} v{}.{}+' but '{}' already required it at v{}.{}+. "
                                        "Multiple distinct major versions of a module cannot "
                                        "coexist in the active graph.",
                                        mod.name, req.target_name,
                                        req.min_major, req.min_minor,
                                        constraint_it->second.requester,
                                        constraint_it->second.min_major,
                                        constraint_it->second.min_minor);
                                }

                                // Same major mismatch but no recorded prior constraint
                                // (explicit activation chose the wrong version). Upgrade.
                                std::size_t better = find_best_module_version(
                                    req.target_name, req.min_major, req.min_minor);

                                if (better == m_modules.size()) {
                                    BOOT_FATAL(ecs,
                                        "[Bootstrapper] Version constraint violation: '{}' "
                                        "requires '{} v{}.{}+' but no staged variant "
                                        "satisfies it.",
                                        mod.name, req.target_name,
                                        req.min_major, req.min_minor);
                                }

                                m_active_modules[req.target_name] = better;
                                m_version_constraints[req.target_name] = {
                                    req.min_major, req.min_minor, mod.name
                                };
                                graph_changed = true;
                            }
                        }

                    } else if (req.target_kind == requirement::kind::service) {
                        // ----- Service dependency --------------------------------
                        if (!is_service_active(req.target_name)) {
                            // Issue 3: find best provider, warn on ambiguity.
                            std::size_t provider_idx = find_best_service_provider(
                                ecs, req.target_name, req.min_major, req.min_minor);

                            if (provider_idx == m_modules.size()) {
                                BOOT_FATAL(ecs,
                                    "[Bootstrapper] Unsatisfied hard service dependency: "
                                    "no staged module provides service '{}' "
                                    "(required by '{}', version window v{}.{}+).",
                                    req.target_name, mod.name,
                                    req.min_major, req.min_minor);
                            }

                            const std::string& provider_name = m_modules[provider_idx].name;
                            m_active_modules[provider_name] = provider_idx;
                            graph_changed = true;
                        }
                    }
                }
            }
        }
    }

    // =========================================================================
    // Private — Phase 2: Service Collision Audit
    // =========================================================================

    void bootstrapper::audit_service_collisions(flecs::world& ecs) {
        // Build a map: service_name → list of { module_name, version, mod_idx }.
        struct candidate {
            std::string  mod_name;
            uint8_t      major;
            uint8_t      minor;
            std::size_t  idx;
        };
        std::unordered_map<std::string, std::vector<candidate>> service_map;

        for (const auto& [mod_name, mod_idx] : m_active_modules) {
            const module_info& mod = m_modules[mod_idx];
            if (mod.provides_service.empty()) continue;
            service_map[mod.provides_service].push_back(
                {mod_name, mod.version_major, mod.version_minor, mod_idx});
        }

        for (auto& [svc_name, candidates] : service_map) {
            if (candidates.size() <= 1) continue;

            // Sort descending by (major, minor) — highest version first (Issue 4).
            std::sort(candidates.begin(), candidates.end(),
                [](const candidate& a, const candidate& b) {
                    if (a.major != b.major) return a.major > b.major;
                    return a.minor > b.minor;
                });

            // Winner is candidates[0]; all others are removed from the active set.
            const candidate& winner = candidates[0];
            for (std::size_t i = 1; i < candidates.size(); ++i) {
                const candidate& loser = candidates[i];
                SANDBOX_WARN(ecs,
                    "[Bootstrapper] Service collision on '{}': module '{}' v{}.{} "
                    "was selected over '{}' v{}.{}. "
                    "The skipped module's ECS component registration has been suppressed "
                    "to prevent an overwrite.",
                    svc_name,
                    winner.mod_name, winner.major, winner.minor,
                    loser.mod_name,  loser.major,  loser.minor);

                m_active_modules.erase(loser.mod_name);
            }
        }
    }

    // =========================================================================
    // Private — Provider Selection Helpers
    // =========================================================================

    std::size_t bootstrapper::find_best_service_provider(
        flecs::world& ecs,
        const std::string& service_name,
        uint8_t min_major,
        uint8_t min_minor) const
    {
        struct candidate { std::size_t idx; uint8_t major; uint8_t minor; };
        std::vector<candidate> matches;

        for (std::size_t i = 0; i < m_modules.size(); ++i) {
            const module_info& mod = m_modules[i];
            if (mod.provides_service != service_name) continue;

            // Version window: major must match exactly (or be unconstrained when
            // min_major == 0), minor must be >= requested.
            const bool major_ok = (min_major == 0) || (mod.version_major == min_major);
            const bool minor_ok = (mod.version_minor >= min_minor);
            if (major_ok && minor_ok) {
                matches.push_back({i, mod.version_major, mod.version_minor});
            }
        }

        if (matches.empty()) return m_modules.size(); // npos

        // Issue 3: sort descending by version, pick highest.
        std::sort(matches.begin(), matches.end(),
            [](const candidate& a, const candidate& b) {
                if (a.major != b.major) return a.major > b.major;
                return a.minor > b.minor;
            });

        if (matches.size() > 1) {
            // Emit the amicable resolution warning (Issue 3).
            const module_info& chosen = m_modules[matches[0].idx];
            SANDBOX_WARN(ecs,
                "[Bootstrapper] Ambiguous service provider for '{}': {} candidate(s) "
                "match the version window. Automatically selected '{}' v{}.{} "
                "(highest available version). Consider pinning a specific provider "
                "to remove this ambiguity.",
                service_name,
                matches.size(),
                chosen.name, chosen.version_major, chosen.version_minor);
        }

        return matches[0].idx;
    }

    std::size_t bootstrapper::find_best_module_version(
        const std::string& module_name,
        uint8_t min_major,
        uint8_t min_minor) const
    {
        struct candidate { std::size_t idx; uint8_t major; uint8_t minor; };
        std::vector<candidate> matches;

        for (std::size_t i = 0; i < m_modules.size(); ++i) {
            const module_info& mod = m_modules[i];
            if (mod.name != module_name) continue;

            const bool major_ok = (min_major == 0) || (mod.version_major == min_major);
            const bool minor_ok = (mod.version_minor >= min_minor);
            if (major_ok && minor_ok) {
                matches.push_back({i, mod.version_major, mod.version_minor});
            }
        }

        if (matches.empty()) return m_modules.size(); // npos

        // Pick the highest version available.
        std::sort(matches.begin(), matches.end(),
            [](const candidate& a, const candidate& b) {
                if (a.major != b.major) return a.major > b.major;
                return a.minor > b.minor;
            });

        return matches[0].idx;
    }

    // =========================================================================
    // Private — Predicate Helpers
    // =========================================================================

    bool bootstrapper::is_module_loaded(const std::string& name) const {
        auto it = m_active_modules.find(name);
        if (it == m_active_modules.end()) return false;
        return m_modules[it->second].is_loaded;
    }

    bool bootstrapper::is_service_loaded(const std::string& service_name) const {
        for (const auto& [mod_name, mod_idx] : m_active_modules) {
            const module_info& mod = m_modules[mod_idx];
            if (mod.provides_service == service_name && mod.is_loaded) return true;
        }
        return false;
    }

    bool bootstrapper::is_service_active(const std::string& service_name) const {
        for (const auto& [mod_name, mod_idx] : m_active_modules) {
            if (m_modules[mod_idx].provides_service == service_name) return true;
        }
        return false;
    }

    bool bootstrapper::all_activated_modules_loaded() const {
        for (const auto& [mod_name, mod_idx] : m_active_modules) {
            if (!m_modules[mod_idx].is_loaded) return false;
        }
        return true;
    }

} // namespace sandbox
