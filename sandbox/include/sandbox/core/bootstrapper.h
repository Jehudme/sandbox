#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_set>

#include "sandbox/core/ecs.h"
#include "sandbox/core/module_info.h"

namespace sandbox {

    struct SANDBOX_API bootstrapper {
    public:
        bootstrapper(flecs::world& ecs) {
            ecs.module<sandbox::bootstrapper>("::Bootstrapper");
            std::cout << "[Bootstrapper] Module initialized.\n";

            std::cout << "[Bootstrapper DEBUG] Initial vector address: " << &modules_infos << "\n";
        }

        ~bootstrapper() = default;

        void stage(const std::vector<module_info>& info) {
            // 1. Print the memory address of the vector to check if we are in the same object
            std::cout << "[Bootstrapper DEBUG] Staging " << info.size()
                      << " modules into vector at address: " << &modules_infos << "\n";

            // 2. Print every module name as it is added
            for (const auto& mod : info) {
                std::cout << "   [Bootstrapper DEBUG] Adding module: '" << mod.name << "'\n";
            }

            modules_infos.insert(modules_infos.end(), info.begin(), info.end());

            // 3. Print the new total size
            std::cout << "[Bootstrapper DEBUG] Total modules now staged: " << modules_infos.size() << "\n";
            std::cout << "[Bootstrapper] staging vector addrr " << &modules_infos << "\n";
        }

        bool activate(const std::string& module_name) {

            std::cout << "[Bootstrapper] activstion vector addrr " << &modules_infos << "\n";

            bool found = false;
            for (const auto& mod : modules_infos ) {
                if (mod.name == module_name) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                std::cerr << "[Bootstrapper] WARNING: Manifest requested '" << module_name
                          << "', but no staged library provides this! Ignoring.\n";
                return false;
            }

            explicit_activations.push_back(module_name);
            std::cout << "[Bootstrapper] Manifest requested activation for: " << module_name << "\n";
            return true;
        }

        void execute(flecs::world& ecs) {
            // Phase 1: Calculate the activation tree
            resolve_activations();

            std::cout << "[Bootstrapper] Executing " << active_module_names.size() << " activated modules...\n";

            // Phase 2: Multi-pass topological sort (ONLY on active modules)
            bool processing = true;
            while (processing) {
                bool progressed_this_cycle = false;

                for (auto& mod : modules_infos) {
                    // SKIP if the module was not activated by the dependency graph
                    if (active_module_names.find(mod.name) == active_module_names.end()) continue;

                    // SKIP if already loaded
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
                        std::cout << " -> Booting module: " << mod.name << "\n";
                        if (mod.import_fn) mod.import_fn(ecs);
                        mod.is_loaded = true;
                        progressed_this_cycle = true;
                    }
                }

                if (!progressed_this_cycle) {
                    if (all_activated_modules_loaded()) {
                        break; // Success!
                    } else {
                        throw std::runtime_error("[Bootstrapper] FATAL: Dependency deadlock detected in activated modules.");
                    }
                }
            }

            std::cout << "[Bootstrapper] All activated modules successfully loaded.\n";
        }

    private:
        std::vector<module_info> modules_infos;
        std::vector<std::string> explicit_activations;
        std::unordered_set<std::string> active_module_names;

        // ====================================================================
        // NEW: Activation Graph Resolver
        // ====================================================================
        void resolve_activations() {
            active_module_names.clear();

            // 1. Seed the graph with explicitly requested modules
            for (const auto& name : explicit_activations) {
                active_module_names.insert(name);
            }

            // 2. Cascade down the requirements until the tree stops growing
            bool graph_changed = true;
            while (graph_changed) {
                graph_changed = false;

                // Loop through all currently active modules to find their requirements
                for (const auto& mod : modules_infos) {
                    if (active_module_names.find(mod.name) != active_module_names.end()) {

                        // Check requirements
                        for (const auto& req : mod.requirements) {
                            if (req.policy == requirement::strictness::require) {

                                // MODULE DEPENDENCY
                                if (req.target_kind == requirement::kind::module) {
                                    if (active_module_names.find(req.target_name) == active_module_names.end()) {
                                        active_module_names.insert(req.target_name);
                                        graph_changed = true;
                                        std::cout << "   [Auto-Activate] Module: " << req.target_name << " (Required by " << mod.name << ")\n";
                                    }
                                }
                                // SERVICE DEPENDENCY
                                else if (req.target_kind == requirement::kind::service) {
                                    if (!is_service_provided_by_active_modules(req.target_name)) {

                                        // We need this service! Search the dormant list for a compatible provider.
                                        module_info* provider = find_dormant_service_provider(req.target_name, req.min_major, req.min_minor);

                                        if (provider) {
                                            active_module_names.insert(provider->name);
                                            graph_changed = true;
                                            std::cout << "   [Auto-Activate] Provider: " << provider->name << " for '" << req.target_name << "' v" << (int)req.min_major << "." << (int)req.min_minor << "\n";
                                        } else {
                                            throw std::runtime_error("[Bootstrapper] FATAL: No provider found for required service: " + req.target_name);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ====================================================================
        // HELPER LOGIC
        // ====================================================================

        bool is_service_provided_by_active_modules(const std::string& service_name) const {
            for (const auto& mod : modules_infos) {
                // If it is active AND provides the service
                if (active_module_names.find(mod.name) != active_module_names.end() && mod.provides_service == service_name) {
                    return true;
                }
            }
            return false;
        }

        // SemVer checking: Major must match exactly, Minor must be >=
        module_info* find_dormant_service_provider(const std::string& service_name, uint8_t min_major, uint8_t min_minor) {
            for (auto& mod : modules_infos) {
                if (mod.provides_service == service_name) {
                    if (mod.version_major == min_major && mod.version_minor >= min_minor) {
                        return &mod;
                    }
                }
            }
            return nullptr;
        }

        bool is_module_loaded(const std::string& name) const {
            for (const auto& mod : modules_infos) {
                if (mod.name == name && mod.is_loaded) return true;
            }
            return false;
        }

        bool is_service_loaded(const std::string& service_name) const {
            for (const auto& mod : modules_infos) {
                if (mod.provides_service == service_name && mod.is_loaded) return true;
            }
            return false;
        }

        // Only checks if ACTIVATED modules are loaded
        bool all_activated_modules_loaded() const {
            for (const auto& mod : modules_infos) {
                if (active_module_names.find(mod.name) != active_module_names.end() && !mod.is_loaded) {
                    return false;
                }
            }
            return true;
        }
    };

} // namespace sandbox