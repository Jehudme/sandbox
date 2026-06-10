#include "sandbox/core/bootstrapper.h"
#include "sandbox/subsystems/logger/ilogger.h"

#include <algorithm>
#include <format>
#include <stdexcept>
#include <queue>

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

        // Kahn's Algorithm for Topological Sorting & Booting
        std::unordered_map<std::string, int> in_degree;
        std::unordered_map<std::string, std::vector<std::string>> graph; // node -> dependents
        
        // 1. Initialize in-degrees to 0 for all active modules
        for (const auto& [mod_name, mod_idx] : m_active_modules) {
            in_degree[mod_name] = 0;
        }

        // 2. Build the graph based on requirements
        for (const auto& [mod_name, mod_idx] : m_active_modules) {
            const module_info& mod = m_modules[mod_idx];
            
            for (const auto& req : mod.requirements) {
                std::string dependency_name;
                
                if (req.target_kind == requirement::kind::module) {
                    if (m_active_modules.count(req.target_name)) {
                        dependency_name = req.target_name;
                    }
                } else if (req.target_kind == requirement::kind::service) {
                    // Find which active module provides this service
                    for (const auto& [active_name, active_idx] : m_active_modules) {
                        if (m_modules[active_idx].provides_service == req.target_name) {
                            dependency_name = active_name;
                            break;
                        }
                    }
                }
                
                if (!dependency_name.empty()) {
                    graph[dependency_name].push_back(mod_name);
                    in_degree[mod_name]++;
                } else if (req.policy == requirement::strictness::require) {
                    // This shouldn't happen as resolve_activations should have caught missing hard dependencies.
                    BOOT_FATAL(ecs, "Fatal: Missing resolved hard dependency for '{}'.", mod_name);
                }
                // If it's an expect and not found, it's safely ignored (in_degree is not incremented).
            }
        }

        // 3. Push modules with in-degree of 0 to a queue
        std::queue<std::string> queue;
        for (const auto& [mod_name, degree] : in_degree) {
            if (degree == 0) {
                queue.push(mod_name);
            }
        }

        // 4. Process queue
        int processed_count = 0;
        while (!queue.empty()) {
            std::string current = queue.front();
            queue.pop();

            module_info& mod = m_modules[m_active_modules[current]];
            if (mod.import_fn) mod.import_fn(ecs);
            mod.is_loaded = true;
            processed_count++;
            
            SANDBOX_INFO(ecs, "[Bootstrapper] Loaded module: {} v{}.{}.{}",
                         mod.name, mod.version_major, mod.version_minor, mod.version_patch);

            for (const std::string& dependent : graph[current]) {
                in_degree[dependent]--;
                if (in_degree[dependent] == 0) {
                    queue.push(dependent);
                }
            }
        }

        // 5. Cycle Detection check
        if (processed_count != m_active_modules.size()) {
            BOOT_FATAL(ecs, "Fatal: Circular dependency detected in module graph");
        }

        SANDBOX_INFO(ecs, "[Bootstrapper] All activated modules loaded successfully.");
    }

    void bootstrapper::resolve_activations(flecs::world& ecs) {
        m_active_modules.clear();
        m_version_constraints.clear();
        
        // Major Version Collision Prevention
        std::unordered_map<std::string, uint8_t> locked_service_majors;

        for (const auto& name : m_explicit_activations) {
            if (m_active_modules.count(name)) continue;

            std::size_t best = find_best_module_version(name, 0, 0);
            if (best == m_modules.size()) {
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
                            // Major Version Collision Prevention for Services
                            if (req.min_major != 0) {
                                auto locked_it = locked_service_majors.find(req.target_name);
                                if (locked_it != locked_service_majors.end()) {
                                    if (locked_it->second != req.min_major) {
                                        if (req.policy == requirement::strictness::require) {
                                            BOOT_FATAL(ecs, "Fatal: Irreconcilable Major Version conflict detected for service {}", req.target_name);
                                        } else {
                                            continue;
                                        }
                                    }
                                } else {
                                    locked_service_majors[req.target_name] = req.min_major;
                                }
                            }

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
