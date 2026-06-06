#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "sandbox/core/ecs.h"
#include "sandbox/core/module_info.h"
#include "sandbox/core/platform.h"

namespace sandbox {

    /// Orchestrates ordered module loading by resolving inter-module dependencies
    /// and service requirements before importing into the ECS world.
    ///
    /// Lifecycle:
    ///   1. stage()   — Append all module descriptors from loaded libraries.
    ///                  Duplicates and multiple versions are allowed at this stage.
    ///   2. activate() — Enqueue an explicit module name from the manifest.
    ///                   Throws std::runtime_error if no staged variant matches.
    ///   3. execute()  — Resolve the full dependency graph, deduplicate versions,
    ///                   audit service collisions, topologically sort, and import.
    struct SANDBOX_API bootstrapper {
    public:
        explicit bootstrapper(flecs::world& ecs);
        ~bootstrapper() = default;

        bootstrapper(const bootstrapper&) = delete;
        bootstrapper& operator=(const bootstrapper&) = delete;
        bootstrapper(bootstrapper&&) = default;
        bootstrapper& operator=(bootstrapper&&) = default;

        /// Appends all module descriptors from a loaded library into the staging pool.
        /// Multiple versions of the same module name may coexist here intentionally.
        void stage(const std::vector<module_info>& info);

        /// Enqueues a module name for explicit activation (must match a staged module).
        /// @throws std::runtime_error if the name is not found in the staging pool.
        void activate(const std::string& module_name);

        /// Resolves the full dependency graph and imports all activated modules in
        /// topological order. Throws on version conflict, deadlock, or unsatisfied
        /// hard dependencies.
        void execute(flecs::world& ecs);

    private:
        // ── Staging pool ─────────────────────────────────────────────────────
        // All staged module_info descriptors — may contain multiple versions of
        // the same logical module name. Never modified after stage() calls end.
        std::vector<module_info> m_modules;

        // ── Activation inputs ─────────────────────────────────────────────────
        // Module names explicitly requested from the manifest.
        std::vector<std::string> m_explicit_activations;

        // ── Resolved active set ───────────────────────────────────────────────
        // Canonical set of module *names* selected to run after graph resolution.
        // Maps module name → index into m_modules of the chosen version.
        std::unordered_map<std::string, std::size_t> m_active_modules;

        // Tracks version constraints applied to each module name during cascade:
        //   key   = module name
        //   value = { min_major, min_minor, name_of_requester }
        // Used to detect conflicting hard-version demands from different branches.
        struct version_constraint {
            uint8_t     min_major;
            uint8_t     min_minor;
            std::string requester;
        };
        std::unordered_map<std::string, version_constraint> m_version_constraints;

        // ── Graph resolution passes ───────────────────────────────────────────

        /// Expands m_active_modules by cascading all strictness::require edges.
        /// Deduplicates versions, detects conflicts, and resolves service providers.
        void resolve_activations(flecs::world& ecs);

        /// Scans the fully resolved active set for duplicate provides_service
        /// registrations. Keeps the highest-version provider; emits SANDBOX_WARN
        /// for every skipped registration.
        void audit_service_collisions(flecs::world& ecs);

        // ── Predicate helpers ─────────────────────────────────────────────────

        /// Returns true if a module with the given name is currently active AND loaded.
        bool is_module_loaded(const std::string& name) const;

        /// Returns true if any active module provides the named service AND is loaded.
        bool is_service_loaded(const std::string& service_name) const;

        /// Returns true if every module in the active set has been imported.
        bool all_activated_modules_loaded() const;

        /// Returns true if any active module provides the named service
        /// (regardless of whether it has been imported yet).
        bool is_service_active(const std::string& service_name) const;

        // ── Provider selection helpers ────────────────────────────────────────

        /// Selects the best-matching staged module index for the given service,
        /// filtering by [min_major, any minor >= min_minor].
        /// When multiple candidates satisfy the window the highest version wins;
        /// a SANDBOX_WARN is emitted for the ambiguous choice.
        /// Returns m_modules.size() (npos) when no candidate qualifies.
        std::size_t find_best_service_provider(
            flecs::world& ecs,
            const std::string& service_name,
            uint8_t min_major,
            uint8_t min_minor) const;

        /// Selects the best-matching staged module index for the given module name
        /// within the provided version window (major exact, minor >=).
        /// Returns m_modules.size() (npos) when no candidate qualifies.
        std::size_t find_best_module_version(
            const std::string& module_name,
            uint8_t min_major,
            uint8_t min_minor) const;
    };

} // namespace sandbox