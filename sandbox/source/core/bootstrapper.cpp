#include "sandbox/core/bootstrapper.h"
#include "sandbox/event_bus/logger_events.h"

#include <stdexcept>

namespace sandbox {

    bootstrapper::bootstrapper(flecs::world& ecs) {
        ecs.module<sandbox::bootstrapper>("::Bootstrapper");
        // No log here; the logger subsystem may not yet be active at this point.
    }

    void bootstrapper::stage(const std::vector<module_info>& info) {
        m_modules.insert(m_modules.end(), info.begin(), info.end());
    }

    bool bootstrapper::activate(const std::string& module_name) {
        for (const auto& mod : m_modules) {
            if (mod.name == module_name) {
                m_explicit_activations.push_back(module_name);
                return true;
            }
        }
        // Module not found among staged libraries — caller should log this.
        return false;
    }

    void bootstrapper::execute(flecs::world& ecs) {
        resolve_activations();

        // Multi-pass topological sort — only processes active modules.
        bool processing = true;
        while (processing) {
            bool progressed_this_cycle = false;

            for (auto& mod : m_modules) {
                if (m_active_module_names.find(mod.name) == m_active_module_names.end()) continue;
                if (mod.is_loaded) continue;

                bool ready_to_boot = true;
                for (const auto& req : mod.requirements) {
                    if (req.policy == requirement::strictness::require) {
                        if (req.target_kind == requirement::kind::module) {
                            if (!is_module_loaded(req.target_name)) { ready_to_boot = false; break; }
                        } else if (req.target_kind == requirement::kind::service) {
                            if (!is_service_loaded(req.target_name)) { ready_to_boot = false; break; }
                        }
                    }
                }

                if (ready_to_boot) {
                    if (mod.import_fn) mod.import_fn(ecs);
                    mod.is_loaded = true;
                    progressed_this_cycle = true;
                    SANDBOX_INFO(ecs, "[Bootstrapper] Loaded module: {}", mod.name);
                }
            }

            if (!progressed_this_cycle) {
                if (all_activated_modules_loaded()) {
                    break;
                } else {
                    throw std::runtime_error(
                        "[Bootstrapper] Dependency deadlock detected: circular or unresolvable requirement in activated modules.");
                }
            }
        }

        SANDBOX_INFO(ecs, "[Bootstrapper] All activated modules loaded.");
    }

    // =========================================================================
    // Private — Dependency Graph Resolution
    // =========================================================================

    void bootstrapper::resolve_activations() {
        m_active_module_names.clear();

        for (const auto& name : m_explicit_activations) {
            m_active_module_names.insert(name);
        }

        // Cascade requirements until the active set stabilises.
        bool graph_changed = true;
        while (graph_changed) {
            graph_changed = false;

            for (const auto& mod : m_modules) {
                if (m_active_module_names.find(mod.name) == m_active_module_names.end()) continue;

                for (const auto& req : mod.requirements) {
                    if (req.policy != requirement::strictness::require) continue;

                    if (req.target_kind == requirement::kind::module) {
                        if (m_active_module_names.find(req.target_name) == m_active_module_names.end()) {
                            m_active_module_names.insert(req.target_name);
                            graph_changed = true;
                        }
                    } else if (req.target_kind == requirement::kind::service) {
                        if (!is_service_provided_by_active_modules(req.target_name)) {
                            module_info* provider = find_dormant_service_provider(req.target_name, req.min_major, req.min_minor);
                            if (provider) {
                                m_active_module_names.insert(provider->name);
                                graph_changed = true;
                            } else {
                                throw std::runtime_error(
                                    "[Bootstrapper] No provider found for required service: '" + req.target_name + "'");
                            }
                        }
                    }
                }
            }
        }
    }

    bool bootstrapper::is_service_provided_by_active_modules(const std::string& service_name) const {
        for (const auto& mod : m_modules) {
            if (m_active_module_names.count(mod.name) && mod.provides_service == service_name) {
                return true;
            }
        }
        return false;
    }

    /// SemVer check: major must match exactly, minor must be >= requested.
    module_info* bootstrapper::find_dormant_service_provider(const std::string& service_name, uint8_t min_major, uint8_t min_minor) {
        for (auto& mod : m_modules) {
            if (mod.provides_service == service_name &&
                mod.version_major == min_major &&
                mod.version_minor >= min_minor)
            {
                return &mod;
            }
        }
        return nullptr;
    }

    bool bootstrapper::is_module_loaded(const std::string& name) const {
        for (const auto& mod : m_modules) {
            if (mod.name == name && mod.is_loaded) return true;
        }
        return false;
    }

    bool bootstrapper::is_service_loaded(const std::string& service_name) const {
        for (const auto& mod : m_modules) {
            if (mod.provides_service == service_name && mod.is_loaded) return true;
        }
        return false;
    }

    bool bootstrapper::all_activated_modules_loaded() const {
        for (const auto& mod : m_modules) {
            if (m_active_module_names.count(mod.name) && !mod.is_loaded) return false;
        }
        return true;
    }

} // namespace sandbox
