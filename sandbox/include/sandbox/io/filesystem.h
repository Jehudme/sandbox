#pragma once

#include "sandbox/core/extension.h"
#include <unordered_map>
#include <string>
#include <flecs.h>

namespace sandbox::extensions
{
    class filesystem : public sandbox::extension
    {
    public:
        // Persistent State
        struct state {
            std::unordered_map<std::string, std::string> mounts;
        };

        // Event: Fired by a system when a file on disk changes
        struct file_changed_event {
            std::string virtual_path;
            std::string physical_path;
        };

        // Request: Add this to an entity to trigger an async file read
        struct read_request { std::string path; };

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;

        std::string resolve(std::string_view path) const;
    };
}