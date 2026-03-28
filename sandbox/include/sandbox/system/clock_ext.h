#pragma once

#include "sandbox/core/extension.h"
#include <flecs.h>

namespace sandbox::extensions
{
    class clock : public sandbox::extension
    {
    public:
        struct state {
            float dt;
            float fixed_dt;
            float total_time;
            float time_scale;
            float accumulator;
        };

        struct fixed_update_tag {};

        struct time_scale_request { float target_scale; float lerp_speed; };

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;
    };
}
