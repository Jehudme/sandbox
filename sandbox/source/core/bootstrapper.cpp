#include "bootstrapper.h"
#include <iostream>
#include <queue>
#include <string>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <format>
#include <unordered_map>
#include <stack> // Changed from <queue> to <stack>

#include <charconv>
#include <sandbox/sdk/logs.hpp>
#include "exceptions.h"
#include <iostream>

namespace sandbox::core {
    auto safe_view = [](const char* str) { return str ? std::string_view(str) : std::string_view(); };

    void bootstrapper_t::reset() {
        // We only clear mock/test plugins from the registry. Real plugins (arch "sandbox")
        // loaded via .so files cannot be safely unloaded and re-staged in the same process
        // since dlclose is merely a hint and global constructors won't re-run.
        m_services.erase(std::remove_if(m_services.begin(), m_services.end(),
            [](const service_info_t& s) { return safe_view(s.architecture) != "sandbox"; }), m_services.end());

        m_modules.erase(std::remove_if(m_modules.begin(), m_modules.end(),
            [](const module_info_t& m) { return safe_view(m.architecture) != "sandbox"; }), m_modules.end());
    }

    void bootstrapper_t::index_library(flecs::world& entity_world, const std::filesystem::path &library_path) {
        m_loader.load(entity_world, library_path);
    }

    void bootstrapper_t::stage_service(const service_info_t& info) {
        auto it = std::find_if(m_services.begin(), m_services.end(), [&](const service_info_t& service) {
            return std::strcmp(service.name, info.name) == 0
                && std::strcmp(service.architecture, info.architecture) == 0
                && service.version_major == info.version_major
                && service.version_minor == info.version_minor;
        });
        if (it == m_services.end()) {
            m_services.push_back(info);
        }
    }

    void bootstrapper_t::stage_module(const module_info_t &info) {
        auto it = std::find_if(m_modules.begin(), m_modules.end(), [&](const module_info_t& module) {
            return std::strcmp(module.name, info.name) == 0
                && std::strcmp(module.architecture, info.architecture) == 0
                && module.version_major == info.version_major
                && module.version_minor == info.version_minor
                && module.version_patch == info.version_patch;
        });
        if (it == m_modules.end()) {
           m_modules.push_back(info);
        }
    }

    static const module_info_t* find_best_module(std::string_view name, std::string_view architecture, int version_major, int version_minor, int version_patch, bool exact_patch, const std::vector<module_info_t>& modules) {
        const module_info_t* best_match = nullptr;
        for (const auto& module_info : modules) {
            if (safe_view(module_info.name) != name) continue;
            if (safe_view(module_info.architecture) != architecture) continue;
            if (version_major > 0 && module_info.version_major != version_major) continue;
            if (version_minor >= 0 && module_info.version_minor < version_minor) continue;
            if (exact_patch && module_info.version_patch != version_patch) continue;

            if (!best_match) {
                best_match = &module_info;
            } else {
                if (module_info.version_major != best_match->version_major) {
                    if (module_info.version_major > best_match->version_major) best_match = &module_info;
                } else if (module_info.version_minor != best_match->version_minor) {
                    if (module_info.version_minor > best_match->version_minor) best_match = &module_info;
                } else if (module_info.version_patch != best_match->version_patch) {
                    if (module_info.version_patch > best_match->version_patch) best_match = &module_info;
                } else if (safe_view(module_info.name) < safe_view(best_match->name)) {
                    best_match = &module_info;
                }
            }
        }
        return best_match;
    }

    static const module_info_t* find_best_service_provider(std::string_view service_name, std::string_view architecture, int version_major, int version_minor, const std::vector<module_info_t>& modules) {
        const module_info_t* best_match = nullptr;
        for (const auto& module_info : modules) {
            if (!module_info.service) continue;
            if (safe_view(module_info.service->name) != service_name) continue;
            if (safe_view(module_info.architecture) != architecture) continue;
            if (version_major > 0 && module_info.service->version_major != version_major) continue;
            if (version_minor >= 0 && module_info.service->version_minor < version_minor) continue;

            if (!best_match) {
                best_match = &module_info;
            } else {
                if (module_info.service->version_major != best_match->service->version_major) {
                    if (module_info.service->version_major > best_match->service->version_major) best_match = &module_info;
                } else if (module_info.service->version_minor != best_match->service->version_minor) {
                    if (module_info.service->version_minor > best_match->service->version_minor) best_match = &module_info;
                } else if (module_info.version_major != best_match->version_major) {
                    if (module_info.version_major > best_match->version_major) best_match = &module_info;
                } else if (module_info.version_minor != best_match->version_minor) {
                    if (module_info.version_minor > best_match->version_minor) best_match = &module_info;
                } else if (module_info.version_patch != best_match->version_patch) {
                    if (module_info.version_patch > best_match->version_patch) best_match = &module_info;
                } else if (safe_view(module_info.name) < safe_view(best_match->name)) {
                    best_match = &module_info;
                }
            }
        }
        return best_match;
    }

    void bootstrapper_t::activate(flecs::world& entity_world, std::string_view module_urn) {
        size_t at_position = module_urn.find('@');
        if (at_position == std::string_view::npos) {
            throw module_activation_error(std::format("Invalid module string format (missing @): {}", module_urn));
        }

        std::string_view full_name = module_urn.substr(0, at_position);
        std::string_view version_str = module_urn.substr(at_position + 1);

        size_t dash_position = full_name.find('-');
        if (dash_position == std::string_view::npos) {
            throw module_activation_error(std::format("Invalid module string format (missing architecture-name separator): {}", module_urn));
        }

        std::string_view architecture = full_name.substr(0, dash_position);
        std::string_view name = full_name.substr(dash_position + 1);

        int version_major = 0, version_minor = 0, version_patch = -1;

        auto parse_integer = [](std::string_view str, int default_value) {
            if (str == "*" || str.empty()) return default_value;
            int value = 0;
            auto result = std::from_chars(str.data(), str.data() + str.size(), value);
            if (result.ec != std::errc()) throw module_activation_error(std::format("Invalid version number: {}", str));
            return value;
        };

        size_t dot_first = version_str.find('.');
        if (dot_first != std::string_view::npos) {
            version_major = parse_integer(version_str.substr(0, dot_first), 0);
            size_t dot_second = version_str.find('.', dot_first + 1);
            if (dot_second != std::string_view::npos) {
                version_minor = parse_integer(version_str.substr(dot_first + 1, dot_second - dot_first - 1), 0);
                version_patch = parse_integer(version_str.substr(dot_second + 1), -1);
            } else {
                version_minor = parse_integer(version_str.substr(dot_first + 1), 0);
            }
        } else {
            version_major = parse_integer(version_str, 0);
        }

        activate(entity_world, architecture, name, version_major, version_minor, version_patch);
    }

    void bootstrapper_t::activate(flecs::world& entity_world, std::string_view architecture, std::string_view name, int version_major, int version_minor, int version_patch) {
        bool exact_patch = (version_patch >= 0);
        const module_info_t* best = find_best_module(name, architecture, version_major, version_minor, version_patch, exact_patch, m_modules);
        if (best) {
            auto is_same_mod = [&](const module_info_t& m) {
                return std::strcmp(m.name, best->name) == 0 && std::strcmp(m.architecture, best->architecture) == 0;
            };

            auto it = std::find_if(m_active_modules.begin(), m_active_modules.end(), is_same_mod);
            auto booted_it = std::find_if(m_booted_modules.begin(), m_booted_modules.end(), is_same_mod);

            if (it == m_active_modules.end() && booted_it == m_booted_modules.end()) {
                m_active_modules.push_back(*best);
                sandbox::modules::logs::trace(entity_world, "Module staged for activation: {} v{}.{} (arch: {})", name, version_major, version_minor, architecture);
            }
        } else {
            std::string error_message = std::format("No matching module found to activate: {} v{}.{} (arch: {})", name, version_major, version_minor, architecture);
            //sandbox::modules::logs::error(entity_world, error_message);
            throw module_activation_error(std::format("No matching module found to activate: {} v{}.{} (arch: {})", name, version_major, version_minor, architecture));
        }
    }

    void bootstrapper_t::boot(flecs::world &entity_world) {
        std::unordered_map<std::string_view, int> service_version_locks;

        // Populate locks with already booted modules across batches
        for (const auto& booted : m_booted_modules) {
            if (booted.service && booted.service->version_major > 0) {
                service_version_locks[booted.service->name] = booted.service->version_major;
            }
        }

        // Loop allows dynamic activations mid-boot to be captured in subsequent batches
        while (!m_active_modules.empty()) {

            // Pass 1: Required dependencies
            size_t processed_index = 0;
            while (processed_index < m_active_modules.size()) {
                module_info_t current = m_active_modules[processed_index++];

                for (size_t i = 0; i < current.requirement_count; ++i) {
                    const auto& requirement = current.requirements[i];
                    if (requirement.strictness != SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED) continue;

                    if (requirement.kind == SANDBOX_REQUIREMENT_KIND_SERVICE) {
                        if (requirement.version_major > 0) {
                            auto lock_it = service_version_locks.find(requirement.name);
                            if (lock_it != service_version_locks.end() && lock_it->second != requirement.version_major) {
                                throw std::runtime_error(std::format("Fatal Major Version Collision for service: {}", requirement.name));
                            }
                            service_version_locks[requirement.name] = requirement.version_major;
                        }

                        bool fulfilled = false;
                        for (const auto& active : m_active_modules) {
                            if (active.service && safe_view(active.service->name) == requirement.name) {
                                fulfilled = true; break;
                            }
                        }
                        if (!fulfilled) {
                            for (const auto& booted : m_booted_modules) {
                                if (booted.service && safe_view(booted.service->name) == requirement.name) {
                                    fulfilled = true; break;
                                }
                            }
                        }
                        if (!fulfilled) {
                            const module_info_t* provider = find_best_service_provider(requirement.name, requirement.architecture, requirement.version_major, requirement.version_minor, m_modules);
                            if (!provider) {
                                throw module_dependency_error(std::format("Required service not found: {}", requirement.name));
                            }
                            sandbox::modules::logs::trace(entity_world, "Auto-pulled provider '{}' for required service: {}", provider->name, requirement.name);
                            m_active_modules.push_back(*provider);
                        }
                    } else if (requirement.kind == SANDBOX_REQUIREMENT_KIND_MODULE) {
                        bool fulfilled = false;
                        for (const auto& active : m_active_modules) {
                            if (safe_view(active.name) == requirement.name) {
                                fulfilled = true; break;
                            }
                        }
                        if (!fulfilled) {
                            for (const auto& booted : m_booted_modules) {
                                if (safe_view(booted.name) == requirement.name) {
                                    fulfilled = true; break;
                                }
                            }
                        }
                        if (!fulfilled) {
                            bool exact = (requirement.version_patch >= 0);
                            const module_info_t* provider = find_best_module(requirement.name, requirement.architecture, requirement.version_major, requirement.version_minor, requirement.version_patch, exact, m_modules);
                            if (!provider) {
                                throw module_dependency_error(std::format("Required module not found: {}", requirement.name));
                            }
                            sandbox::modules::logs::trace(entity_world, "Auto-pulled module '{}' to satisfy dependency for: {}", provider->name, current.name);
                            m_active_modules.push_back(*provider);
                        }
                    }
                }
            }

            // Pass 2: Expected dependencies
            processed_index = 0;
            while (processed_index < m_active_modules.size()) {
                module_info_t current = m_active_modules[processed_index++];
                for (size_t i = 0; i < current.requirement_count; ++i) {
                    const auto& requirement = current.requirements[i];
                    if (requirement.strictness != SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED) continue;

                    if (requirement.kind == SANDBOX_REQUIREMENT_KIND_SERVICE) {
                        if (requirement.version_major > 0) {
                            auto lock_it = service_version_locks.find(requirement.name);
                            if (lock_it != service_version_locks.end() && lock_it->second != requirement.version_major) {
                                throw service_collision_error(std::format("Fatal Major Version Collision for service: {}", requirement.name));
                            }
                            service_version_locks[requirement.name] = requirement.version_major;
                        }

                        bool fulfilled = false;
                        for (const auto& active : m_active_modules) {
                            if (active.service && safe_view(active.service->name) == requirement.name) {
                                fulfilled = true; break;
                            }
                        }
                        if (!fulfilled) {
                            for (const auto& booted : m_booted_modules) {
                                if (booted.service && safe_view(booted.service->name) == requirement.name) {
                                    fulfilled = true; break;
                                }
                            }
                        }
                        if (!fulfilled) {
                            const module_info_t* provider = find_best_service_provider(requirement.name, requirement.architecture, requirement.version_major, requirement.version_minor, m_modules);
                            if (provider) {
                                m_active_modules.push_back(*provider);
                            }
                        }
                    } else if (requirement.kind == SANDBOX_REQUIREMENT_KIND_MODULE) {
                        bool fulfilled = false;
                        for (const auto& active : m_active_modules) {
                            if (safe_view(active.name) == requirement.name) {
                                fulfilled = true; break;
                            }
                        }
                        if (!fulfilled) {
                            for (const auto& booted : m_booted_modules) {
                                if (safe_view(booted.name) == requirement.name) {
                                    fulfilled = true; break;
                                }
                            }
                        }
                        if (!fulfilled) {
                            bool exact = (requirement.version_patch >= 0);
                            const module_info_t* provider = find_best_module(requirement.name, requirement.architecture, requirement.version_major, requirement.version_minor, requirement.version_patch, exact, m_modules);
                            if (provider) {
                                m_active_modules.push_back(*provider);
                            }
                        }
                    }
                }
            }

            // 3. Service Collision Auditing
            std::unordered_map<std::string_view, std::vector<size_t>> service_providers;
            for (size_t i = 0; i < m_active_modules.size(); ++i) {
                if (m_active_modules[i].service) {
                    service_providers[m_active_modules[i].service->name].push_back(i);
                }
            }

            std::vector<size_t> to_evict;
            for (const auto& pair : service_providers) {
                const auto& indices = pair.second;
                if (indices.size() > 1) {
                    size_t winner_idx = indices[0];
                    for (size_t i = 1; i < indices.size(); ++i) {
                        const module_info_t& current = m_active_modules[indices[i]];
                        const module_info_t& winner = m_active_modules[winner_idx];

                        bool current_wins = false;
                        if (current.version_major != winner.version_major) current_wins = current.version_major > winner.version_major;
                        else if (current.version_minor != winner.version_minor) current_wins = current.version_minor > winner.version_minor;
                        else if (current.version_patch != winner.version_patch) current_wins = current.version_patch > winner.version_patch;
                        else current_wins = safe_view(current.name) < safe_view(winner.name);

                        if (current_wins) {
                            to_evict.push_back(winner_idx);
                            winner_idx = indices[i];
                        } else {
                            to_evict.push_back(indices[i]);
                        }
                    }
                }
            }

            std::sort(to_evict.begin(), to_evict.end(), std::greater<size_t>());
            for (size_t idx : to_evict) {
                sandbox::modules::logs::warn(entity_world, "ECS registration suppressed for module '{}' due to service collision on '{}'",
                                             m_active_modules[idx].name, m_active_modules[idx].service->name);
                m_active_modules.erase(m_active_modules.begin() + idx);
            }

            // Extract the current batch. This clears m_active_modules so that
            // any init_fn calling activate() will safely stage modules for the NEXT while iteration.
            std::vector<module_info_t> current_batch = std::move(m_active_modules);
            m_active_modules.clear();

            // 4. Execution Phase: Kahn's Algorithm on current_batch
            std::vector<std::vector<size_t>> adj(current_batch.size());
            std::vector<int> in_degree(current_batch.size(), 0);

            std::unordered_map<std::string_view, size_t> batch_module_names;
            std::unordered_map<std::string_view, size_t> batch_service_names;
            for (size_t j = 0; j < current_batch.size(); ++j) {
                batch_module_names[safe_view(current_batch[j].name)] = j;
                if (current_batch[j].service) {
                    batch_service_names[safe_view(current_batch[j].service->name)] = j;
                }
            }

            for (size_t a = 0; a < current_batch.size(); ++a) {
                const module_info_t& mod_a = current_batch[a];
                for (size_t i = 0; i < mod_a.requirement_count; ++i) {
                    const auto& requirement = mod_a.requirements[i];

                    size_t b = current_batch.size();
                    if (requirement.kind == SANDBOX_REQUIREMENT_KIND_SERVICE) {
                        auto it = batch_service_names.find(safe_view(requirement.name));
                        if (it != batch_service_names.end()) {
                            b = it->second;
                        }
                    } else if (requirement.kind == SANDBOX_REQUIREMENT_KIND_MODULE) {
                        auto it = batch_module_names.find(safe_view(requirement.name));
                        if (it != batch_module_names.end()) {
                            b = it->second;
                        }
                    }

                    // If 'b' is found in this batch, add edge.
                    // (If not found, it implies it was already resolved in a prior batch and booted).
                    if (b < current_batch.size()) {
                        // b must initialize before a
                        adj[b].push_back(a);
                        in_degree[a]++;
                    }
                }
            }

            // Using a queue yields breadth-first dependency initialization
            std::queue<size_t> s;
            for (size_t i = 0; i < current_batch.size(); ++i) {
                if (in_degree[i] == 0) {
                    s.push(i);
                }
            }

            while (!s.empty()) {
                size_t curr = s.front();
                s.pop();

                if (current_batch[curr].init_fn) {
                    std::cout << "[Bootstrapper] Booting module: " << current_batch[curr].name << "\n";
                    current_batch[curr].init_fn(entity_world.c_ptr());
                }
                m_booted_modules.push_back(current_batch[curr]);

                for (size_t dependent : adj[curr]) {
                    in_degree[dependent]--;
                    if (in_degree[dependent] == 0) {
                        s.push(dependent);
                    }
                }
            }

            for (size_t i = 0; i < in_degree.size(); i++) {
                if (in_degree[i] != 0) {
                    throw module_dependency_error("Cyclic dependency detected during module boot.");
                }
            }
        }
    }
}