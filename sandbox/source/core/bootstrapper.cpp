#include "bootstrapper.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <format>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <charconv>

namespace sandbox::core {

    void bootstrapper_t::reset() {
        m_services.clear();
        m_modules.clear();
        m_loader = library_loader_t{};
    }

    void bootstrapper_t::index_library(const std::filesystem::path &library_path) {
        m_loader.load(library_path);
    }

    void bootstrapper_t::stage_service(const service_info_t& info) {
        auto it = std::find_if(m_services.begin(), m_services.end(), [&](const service_info_t& service) {
            return std::strcmp(service.name, info.name) == 0
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
                && module.version_major == info.version_major
                && module.version_minor == info.version_minor
                && module.version_patch == info.version_patch;
        });
        if (it == m_modules.end()) {
           m_modules.push_back(info);
        }
    }

    static const module_info_t* find_best_module(std::string_view name, std::string_view arch, int v_maj, int v_min, int v_patch, bool exact_patch, const std::vector<module_info_t>& modules) {
        const module_info_t* best = nullptr;
        for (const auto& mod : modules) {
            if (std::string_view(mod.name) != name) continue;
            if (std::string_view(mod.architecture) != arch) continue;
            if (v_maj > 0 && mod.version_major != v_maj) continue;
            if (v_min >= 0 && mod.version_minor < v_min) continue;
            if (exact_patch && mod.version_patch != v_patch) continue;
            
            if (!best) {
                best = &mod;
            } else {
                if (mod.version_major != best->version_major) {
                    if (mod.version_major > best->version_major) best = &mod;
                } else if (mod.version_minor != best->version_minor) {
                    if (mod.version_minor > best->version_minor) best = &mod;
                } else if (mod.version_patch != best->version_patch) {
                    if (mod.version_patch > best->version_patch) best = &mod;
                } else if (std::string_view(mod.name) < std::string_view(best->name)) {
                    best = &mod;
                }
            }
        }
        return best;
    }

    static const module_info_t* find_best_service_provider(std::string_view srv_name, std::string_view arch, int v_maj, int v_min, const std::vector<module_info_t>& modules) {
        const module_info_t* best = nullptr;
        for (const auto& mod : modules) {
            if (!mod.service) continue;
            if (std::string_view(mod.service->name) != srv_name) continue;
            if (std::string_view(mod.architecture) != arch) continue;
            if (v_maj > 0 && mod.service->version_major != v_maj) continue;
            if (v_min >= 0 && mod.service->version_minor < v_min) continue;
            
            if (!best) {
                best = &mod;
            } else {
                if (mod.service->version_major != best->service->version_major) {
                    if (mod.service->version_major > best->service->version_major) best = &mod;
                } else if (mod.service->version_minor != best->service->version_minor) {
                    if (mod.service->version_minor > best->service->version_minor) best = &mod;
                } else if (mod.version_major != best->version_major) {
                    if (mod.version_major > best->version_major) best = &mod;
                } else if (mod.version_minor != best->version_minor) {
                    if (mod.version_minor > best->version_minor) best = &mod;
                } else if (mod.version_patch != best->version_patch) {
                    if (mod.version_patch > best->version_patch) best = &mod;
                } else if (std::string_view(mod.name) < std::string_view(best->name)) {
                    best = &mod;
                }
            }
        }
        return best;
    }

    void bootstrapper_t::activate(std::string_view module_str) {
        size_t at_pos = module_str.find('@');
        if (at_pos == std::string_view::npos) {
            throw std::invalid_argument(std::format("Invalid module string format (missing @): {}", module_str));
        }
        
        std::string_view arch_name = module_str.substr(0, at_pos);
        std::string_view version_str = module_str.substr(at_pos + 1);
        
        size_t dash_pos = arch_name.rfind('-');
        if (dash_pos == std::string_view::npos) {
            throw std::invalid_argument(std::format("Invalid module string format (missing architecture-name separator): {}", module_str));
        }
        
        std::string_view architecture = arch_name.substr(0, dash_pos);
        std::string_view name = arch_name.substr(dash_pos + 1);
        
        int v_major = 0, v_minor = 0, v_patch = -1;
        
        auto parse_int = [](std::string_view str, int default_val) {
            if (str == "*" || str.empty()) return default_val;
            int val = 0;
            auto result = std::from_chars(str.data(), str.data() + str.size(), val);
            if (result.ec != std::errc()) throw std::invalid_argument(std::format("Invalid version number: {}", str));
            return val;
        };
        
        size_t dot1 = version_str.find('.');
        if (dot1 != std::string_view::npos) {
            v_major = parse_int(version_str.substr(0, dot1), 0);
            size_t dot2 = version_str.find('.', dot1 + 1);
            if (dot2 != std::string_view::npos) {
                v_minor = parse_int(version_str.substr(dot1 + 1, dot2 - dot1 - 1), 0);
                v_patch = parse_int(version_str.substr(dot2 + 1), -1);
            } else {
                v_minor = parse_int(version_str.substr(dot1 + 1), 0);
            }
        } else {
            v_major = parse_int(version_str, 0);
        }
        
        activate(architecture, name, v_major, v_minor, v_patch);
    }

    void bootstrapper_t::activate(std::string_view architecture, std::string_view name, int version_major, int version_minor, int version_patch) {
        bool exact_patch = (version_patch >= 0);
        const module_info_t* best = find_best_module(name, architecture, version_major, version_minor, version_patch, exact_patch, m_modules);
        if (best) {
            auto it = std::find_if(m_active_modules.begin(), m_active_modules.end(), [&](const module_info_t& active) {
                return std::strcmp(active.name, best->name) == 0 && std::strcmp(active.architecture, best->architecture) == 0;
            });
            if (it == m_active_modules.end()) {
                m_active_modules.push_back(*best);
            }
        } else {
            throw std::invalid_argument(std::format("No matching module found to activate: {} v{}.{} (arch: {})", name, version_major, version_minor, architecture));
        }
    }

    void bootstrapper_t::boot(flecs::world &ecs) {
        std::unordered_map<std::string_view, int> service_version_locks;
        
        // Pass 1: Required dependencies
        size_t processed_index = 0;
        while (processed_index < m_active_modules.size()) {
            const module_info_t& current = m_active_modules[processed_index++];
            
            for (size_t i = 0; i < current.requirement_count; ++i) {
                const auto& req = current.requirements[i];
                if (req.strictness != SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED) continue;
                
                if (req.kind == SANDBOX_REQUIREMENT_KIND_SERVICE) {
                    if (req.version_major > 0) {
                        auto lock_it = service_version_locks.find(req.name);
                        if (lock_it != service_version_locks.end() && lock_it->second != req.version_major) {
                            throw std::runtime_error(std::format("Fatal Major Version Collision for service: {}", req.name));
                        }
                        service_version_locks[req.name] = req.version_major;
                    }
                    
                    bool fulfilled = false;
                    for (const auto& active : m_active_modules) {
                        if (active.service && std::string_view(active.service->name) == req.name) {
                            fulfilled = true;
                            break;
                        }
                    }
                    if (!fulfilled) {
                        const module_info_t* provider = find_best_service_provider(req.name, req.architecture, req.version_major, req.version_minor, m_modules);
                        if (!provider) {
                            throw std::runtime_error(std::format("Required service not found: {}", req.name));
                        }
                        m_active_modules.push_back(*provider);
                    }
                } else if (req.kind == SANDBOX_REQUIREMENT_KIND_MODULE) {
                    bool fulfilled = false;
                    for (const auto& active : m_active_modules) {
                        if (std::string_view(active.name) == req.name) {
                            fulfilled = true;
                            break;
                        }
                    }
                    if (!fulfilled) {
                        bool exact = (req.version_patch >= 0);
                        const module_info_t* provider = find_best_module(req.name, req.architecture, req.version_major, req.version_minor, req.version_patch, exact, m_modules);
                        if (!provider) {
                            throw std::runtime_error(std::format("Required module not found: {}", req.name));
                        }
                        m_active_modules.push_back(*provider);
                    }
                }
            }
        }
        
        // Pass 2: Expected dependencies
        processed_index = 0;
        while (processed_index < m_active_modules.size()) {
            const module_info_t& current = m_active_modules[processed_index++];
            for (size_t i = 0; i < current.requirement_count; ++i) {
                const auto& req = current.requirements[i];
                if (req.strictness != SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED) continue;
                
                if (req.kind == SANDBOX_REQUIREMENT_KIND_SERVICE) {
                    if (req.version_major > 0) {
                        auto lock_it = service_version_locks.find(req.name);
                        if (lock_it != service_version_locks.end() && lock_it->second != req.version_major) {
                            throw std::runtime_error(std::format("Fatal Major Version Collision for service: {}", req.name));
                        }
                        service_version_locks[req.name] = req.version_major;
                    }
                    
                    bool fulfilled = false;
                    for (const auto& active : m_active_modules) {
                        if (active.service && std::string_view(active.service->name) == req.name) {
                            fulfilled = true;
                            break;
                        }
                    }
                    if (!fulfilled) {
                        const module_info_t* provider = find_best_service_provider(req.name, req.architecture, req.version_major, req.version_minor, m_modules);
                        if (provider) {
                            m_active_modules.push_back(*provider);
                        }
                    }
                } else if (req.kind == SANDBOX_REQUIREMENT_KIND_MODULE) {
                    bool fulfilled = false;
                    for (const auto& active : m_active_modules) {
                        if (std::string_view(active.name) == req.name) {
                            fulfilled = true;
                            break;
                        }
                    }
                    if (!fulfilled) {
                        bool exact = (req.version_patch >= 0);
                        const module_info_t* provider = find_best_module(req.name, req.architecture, req.version_major, req.version_minor, req.version_patch, exact, m_modules);
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
                    else current_wins = std::string_view(current.name) < std::string_view(winner.name);
                    
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
            std::cout << std::format("[Warning] ECS registration suppressed for module '{}' due to service collision on '{}'\n", 
                                     m_active_modules[idx].name, m_active_modules[idx].service->name);
            m_active_modules.erase(m_active_modules.begin() + idx);
        }
        
        // 4. Execution Phase: Kahn's Algorithm
        std::vector<std::vector<size_t>> adj(m_active_modules.size());
        std::vector<int> in_degree(m_active_modules.size(), 0);
        
        for (size_t a = 0; a < m_active_modules.size(); ++a) {
            const module_info_t& mod_a = m_active_modules[a];
            for (size_t i = 0; i < mod_a.requirement_count; ++i) {
                const auto& req = mod_a.requirements[i];
                
                size_t b = m_active_modules.size();
                if (req.kind == SANDBOX_REQUIREMENT_KIND_SERVICE) {
                    for (size_t j = 0; j < m_active_modules.size(); ++j) {
                        if (m_active_modules[j].service && std::string_view(m_active_modules[j].service->name) == req.name) {
                            b = j;
                            break;
                        }
                    }
                } else if (req.kind == SANDBOX_REQUIREMENT_KIND_MODULE) {
                    for (size_t j = 0; j < m_active_modules.size(); ++j) {
                        if (std::string_view(m_active_modules[j].name) == req.name) {
                            b = j;
                            break;
                        }
                    }
                }
                
                if (b < m_active_modules.size()) {
                    // b must initialize before a
                    adj[b].push_back(a);
                    in_degree[a]++;
                }
            }
        }
        
        std::queue<size_t> q;
        for (size_t i = 0; i < m_active_modules.size(); ++i) {
            if (in_degree[i] == 0) {
                q.push(i);
            }
        }
        
        size_t initialized_count = 0;
        std::vector<size_t> boot_order;
        while (!q.empty()) {
            size_t curr = q.front();
            q.pop();
            boot_order.push_back(curr);
            
            if (m_active_modules[curr].init_fn) {
                m_active_modules[curr].init_fn(ecs.c_ptr());
            }
            initialized_count++;
            
            for (size_t dependent : adj[curr]) {
                in_degree[dependent]--;
                if (in_degree[dependent] == 0) {
                    q.push(dependent);
                }
            }
        }
        
        if (initialized_count != m_active_modules.size()) {
            throw std::runtime_error("Cyclic dependency detected during module boot.");
        }
        
        m_active_modules.clear();
    }
}
