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
            float accumulator; // For fixed-step automation
        };

        // Tag: Add this to a system to make it run on the FixedUpdate pulse
        struct fixed_update_tag {};

        // Request: Add this to the world to change time scale smoothly
        struct time_scale_request { float target_scale; float lerp_speed; };

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;
    };
}