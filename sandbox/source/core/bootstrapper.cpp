#include "sandbox/core/bootstrapper.h"
#include "sandbox/event_bus/logger_events.h"

#include <algorithm>
#include <format>
#include <stdexcept>

#define BOOT_FATAL(world, fmt_str, ...) \
    do { \
        std::string _boot_msg = std::format(fmt_str, ##__VA_ARGS__); \
        SANDBOX_ERROR(world, "{}", _boot_msg); \
        throw std::runtime_error(_boot_msg); \
    } while(0)

namespace sandbox {

    bootstrapper::bootstrapper(flecs::world& ecs) {
        ecs.module<sandbox::bootstrapper>("::Bootstrapper");
    }

    void bootstrapper::stage(const library_registry& registry) {
        m_modules.insert(m_modules.end(), registry.modules.begin(), registry.modules.end());
        m_services.insert(m_services.end(), registry.services.begin(), registry.services.end());
    }

    void bootstrapper::activate(const std::string& module_name) {
        for (const auto& mod : m_modules) {
            if (mod.name == module_name) {
                m_explicit_activations.push_back(module_name);
                return;
            }
        }
        throw std::runtime_error(
            "[Bootstrapper] activate() failed: module '" + module_name +
            "' was not found in any staged library. "
            "Ensure the library is loaded before calling execute().");
    }

    void bootstrapper::execute(flecs::world& ecs) {
        resolve_activations(ecs);
        audit_service_collisions(ecs);

        bool processing = true;
        while (processing) {
            bool progressed_this_cycle = false;

            for (auto& [mod_name, mod_idx] : m_active_modules) {
                module_info& mod = m_modules[mod_idx];
                if (mod.is_loaded) continue;

                bool ready_to_boot = true;
                for (const auto& req : mod.requirements) {
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
                    mod.is_loaded = true;
                    progressed_this_cycle = true;
                    SANDBOX_INFO(ecs, "[Bootstrapper] Loaded module: {} v{}.{}.{}",
                                 mod.name, mod.version_major, mod.version_minor, mod.version_patch);
                }
            }

            if (!progressed_this_cycle) {
                if (all_activated_modules_loaded()) {
                    break;
                }
                BOOT_FATAL(ecs,
                    "[Bootstrapper] Dependency deadlock detected: circular or "
                    "unresolvable hard requirement in the activated module graph.");
            }
        }
        SANDBOX_INFO(ecs, "[Bootstrapper] All activated modules loaded successfully.");
    }

    void bootstrapper::resolve_activations(flecs::world& ecs) {
        m_active_modules.clear();
        m_version_constraints.clear();

        for (const auto& name : m_explicit_activations) {
            if (m_active_modules.count(name)) continue;

            const std::size_t npos = m_modules.size();
            std::size_t best = find_best_module_version(name, 0, 0);
            if (best == npos) {
                BOOT_FATAL(ecs,
                    "[Bootstrapper] Explicit activation '{}' has no staged variant "
                    "(internal inconsistency — was stage() called before execute()?).", name);
            }
            m_active_modules[name] = best;
        }

        auto resolve_pass = [&](requirement::strictness filter_policy) -> bool {
            bool pass_changed = false;
            bool graph_changed = true;
            while (graph_changed) {
                graph_changed = false;

                std::vector<std::string> current_names;
                current_names.reserve(m_active_modules.size());
                for (const auto& [k, _] : m_active_modules) current_names.push_back(k);

                for (const auto& active_name : current_names) {
                    const module_info& mod = m_modules[m_active_modules.at(active_name)];

                    for (const auto& req : mod.requirements) {
                        // In the first pass we ONLY do require.
                        // In the second pass, we do expect (and if they add modules, we also do their requires).
                        // So if filter_policy == require, we only evaluate requires.
                        // If filter_policy == expect, we evaluate BOTH requires and expects.
                        if (filter_policy == requirement::strictness::require && req.policy != requirement::strictness::require) continue;

                        if (req.target_kind == requirement::kind::module) {
                            auto it = m_active_modules.find(req.target_name);
                            if (it == m_active_modules.end()) {
                                std::size_t best = find_best_module_version(req.target_name, req.min_major, req.min_minor);
                                if (best == m_modules.size()) {
                                    if (req.policy == requirement::strictness::require) {
                                        BOOT_FATAL(ecs,
                                            "[Bootstrapper] Unsatisfied hard module dependency: "
                                            "'{}' (required by '{}') has no staged variant matching v{}.{}+.",
                                            req.target_name, mod.name, req.min_major, req.min_minor);
                                    } else {
                                        continue; // Soft dependency unmet, just skip
                                    }
                                }

                                m_active_modules[req.target_name] = best;
                                m_version_constraints[req.target_name] = {req.min_major, req.min_minor, mod.name};
                                graph_changed = true;
                                pass_changed = true;
                            } else {
                                const module_info& chosen = m_modules[it->second];
                                const bool major_ok = (chosen.version_major == req.min_major);
                                const bool minor_ok = (chosen.version_minor >= req.min_minor);
                                const bool zero_req = (req.min_major == 0);

                                if (!zero_req && !(major_ok && minor_ok)) {
                                    auto constraint_it = m_version_constraints.find(req.target_name);
                                    if (constraint_it != m_version_constraints.end() &&
                                        constraint_it->second.min_major != req.min_major)
                                    {
                                        if (req.policy == requirement::strictness::require) {
                                            BOOT_FATAL(ecs,
                                                "[Bootstrapper] Fatal version conflict: '{}' requires '{} v{}.{}+' "
                                                "but '{}' already required it at v{}.{}+.",
                                                mod.name, req.target_name, req.min_major, req.min_minor,
                                                constraint_it->second.requester,
                                                constraint_it->second.min_major, constraint_it->second.min_minor);
                                        } else {
                                            continue; // Conflict on expect, just ignore the expect.
                                        }
                                    }

                                    std::size_t better = find_best_module_version(req.target_name, req.min_major, req.min_minor);
                                    if (better == m_modules.size()) {
                                        if (req.policy == requirement::strictness::require) {
                                            BOOT_FATAL(ecs,
                                                "[Bootstrapper] Version constraint violation: '{}' requires '{} v{}.{}+' "
                                                "but no staged variant satisfies it.",
                                                mod.name, req.target_name, req.min_major, req.min_minor);
                                        } else {
                                            continue;
                                        }
                                    }

                                    m_active_modules[req.target_name] = better;
                                    m_version_constraints[req.target_name] = {req.min_major, req.min_minor, mod.name};
                                    graph_changed = true;
                                    pass_changed = true;
                                }
                            }
                        } else if (req.target_kind == requirement::kind::service) {
                            if (!is_service_active(req.target_name)) {
                                std::size_t provider_idx = find_best_service_provider(ecs, req.target_name, req.min_major, req.min_minor);
                                if (provider_idx == m_modules.size()) {
                                    if (req.policy == requirement::strictness::require) {
                                        BOOT_FATAL(ecs,
                                            "[Bootstrapper] Unsatisfied hard service dependency: "
                                            "no staged module provides service '{}' (required by '{}', version window v{}.{}+).",
                                            req.target_name, mod.name, req.min_major, req.min_minor);
                                    } else {
                                        continue;
                                    }
                                }

                                const std::string& provider_name = m_modules[provider_idx].name;
                                m_active_modules[provider_name] = provider_idx;
                                graph_changed = true;
                                pass_changed = true;
                            }
                        }
                    }
                }
            }
            return pass_changed;
        };

        // First Pass: Resolve strict require dependencies
        resolve_pass(requirement::strictness::require);

        // Second Pass: Evaluate soft expect dependencies
        resolve_pass(requirement::strictness::expect);
    }

    void bootstrapper::audit_service_collisions(flecs::world& ecs) {
        struct candidate {
            std::string mod_name;
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
            std::size_t idx;
        };
        std::unordered_map<std::string, std::vector<candidate>> service_map;

        for (const auto& [mod_name, mod_idx] : m_active_modules) {
            const module_info& mod = m_modules[mod_idx];
            if (mod.provides_service.empty()) continue;
            service_map[mod.provides_service].push_back(
                {mod_name, mod.version_major, mod.version_minor, mod.version_patch, mod_idx});
        }

        for (auto& [svc_name, candidates] : service_map) {
            if (candidates.size() <= 1) continue;

            std::sort(candidates.begin(), candidates.end(),
                [](const candidate& a, const candidate& b) {
                    if (a.major != b.major) return a.major > b.major;
                    if (a.minor != b.minor) return a.minor > b.minor;
                    return a.patch > b.patch;
                });

            const candidate& winner = candidates[0];
            for (std::size_t i = 1; i < candidates.size(); ++i) {
                const candidate& loser = candidates[i];
                SANDBOX_WARN(ecs,
                    "[Bootstrapper] Service collision on '{}': module '{}' v{}.{}.{} "
                    "was selected over '{}' v{}.{}.{}. "
                    "The skipped module's ECS component registration has been suppressed.",
                    svc_name,
                    winner.mod_name, winner.major, winner.minor, winner.patch,
                    loser.mod_name, loser.major, loser.minor, loser.patch);
                m_active_modules.erase(loser.mod_name);
            }
        }
    }

    std::size_t bootstrapper::find_best_service_provider(
        flecs::world& ecs,
        const std::string& service_name,
        uint8_t min_major,
        uint8_t min_minor) const
    {
        struct candidate { std::size_t idx; uint8_t major; uint8_t minor; uint8_t patch; std::string name; };
        std::vector<candidate> matches;

        for (std::size_t i = 0; i < m_modules.size(); ++i) {
            const module_info& mod = m_modules[i];
            if (mod.provides_service != service_name) continue;

            const bool major_ok = (min_major == 0) || (mod.version_major == min_major);
            const bool minor_ok = (mod.version_minor >= min_minor);
            if (major_ok && minor_ok) {
                matches.push_back({i, mod.version_major, mod.version_minor, mod.version_patch, mod.name});
            }
        }

        if (matches.empty()) return m_modules.size();

        std::sort(matches.begin(), matches.end(),
            [](const candidate& a, const candidate& b) {
                if (a.major != b.major) return a.major > b.major;
                if (a.minor != b.minor) return a.minor > b.minor;
                if (a.patch != b.patch) return a.patch > b.patch;
                return a.name < b.name; // deterministic choice for ambiguous providers
            });

        if (matches.size() > 1) {
            bool ambiguous = false;
            if (matches[0].name != matches[1].name && matches[0].major == matches[1].major && matches[0].minor == matches[1].minor && matches[0].patch == matches[1].patch) {
                ambiguous = true;
            }
            if (ambiguous) {
                const module_info& chosen = m_modules[matches[0].idx];
                SANDBOX_WARN(ecs,
                    "[Bootstrapper] Ambiguous service provider for '{}': distinct candidates "
                    "match exactly. Automatically selected '{}' v{}.{}.{} deterministically. "
                    "Consider pinning a specific provider.",
                    service_name, chosen.name, chosen.version_major, chosen.version_minor, chosen.version_patch);
            }
        }

        return matches[0].idx;
    }

    std::size_t bootstrapper::find_best_module_version(
        const std::string& module_name,
        uint8_t min_major,
        uint8_t min_minor) const
    {
        struct candidate { std::size_t idx; uint8_t major; uint8_t minor; uint8_t patch; };
        std::vector<candidate> matches;

        for (std::size_t i = 0; i < m_modules.size(); ++i) {
            const module_info& mod = m_modules[i];
            if (mod.name != module_name) continue;

            const bool major_ok = (min_major == 0) || (mod.version_major == min_major);
            const bool minor_ok = (mod.version_minor >= min_minor);
            if (major_ok && minor_ok) {
                matches.push_back({i, mod.version_major, mod.version_minor, mod.version_patch});
            }
        }

        if (matches.empty()) return m_modules.size();

        std::sort(matches.begin(), matches.end(),
            [](const candidate& a, const candidate& b) {
                if (a.major != b.major) return a.major > b.major;
                if (a.minor != b.minor) return a.minor > b.minor;
                return a.patch > b.patch;
            });

        return matches[0].idx;
    }

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
