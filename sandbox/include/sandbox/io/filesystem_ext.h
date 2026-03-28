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
        struct state {
            std::unordered_map<std::string, std::string> mounts;
        };

        struct file_changed_event {
            std::string virtual_path;
            std::string physical_path;
        };

        struct read_request { std::string path; };

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;

        std::string resolve(std::string_view path) const;
    };
}
