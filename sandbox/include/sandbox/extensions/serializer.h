#pragma once

#include "sandbox/core/extension.h"
#include <flecs.h>
#include <string>

namespace sandbox::extensions
{
    class serializer : public sandbox::extension
    {
    public:
        struct config {
            std::string root_dir;
        };

        // Request: Add this to an entity to trigger an auto-save to JSON
        struct save_request { std::string filename; };

        // Request: Add this to a new entity to "hydrate" it from a file
        struct load_request { std::string filename; };

        // Tag: Entities with this are automatically saved when they change
        struct persistent_tag {};

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;
    };
}

#include "serializer.inl"