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

        struct save_request { std::string filename; };

        struct load_request { std::string filename; };

        struct persistent_tag {};

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;
    };
}

#include "serializer_ext.inl"
