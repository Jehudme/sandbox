#include <sandbox/sandbox.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "sandbox/core/engine.h"
#include "sandbox/reflections/registration.h"

struct position
{
    float x;
    float y;
};

void my_global_function()
{
    SANDBOX_LOG_INFO("Registry: Global function invoked successfully!");
}

struct math_utility
{
    static int multiply(int a, int b)
    {
        return a * b;
    }
};

class test_extension : public sandbox::extension
{
public:
    void initialize(sandbox::engine& app, const sandbox::properties& props) override
    {
        SANDBOX_LOG_INFO("Extension 'test_extension' initialized.");

        app.create_system(
            "timer_system",
            "",
            [](auto& builder) {
                builder.interval(5.0f);
            },
            [](flecs::iter& it) {
                SANDBOX_LOG_INFO("--> Extension: 5 seconds have elapsed!");
            }
        );
    }

    void finalize(sandbox::engine& app) override
    {
        SANDBOX_LOG_INFO("Extension 'test_extension' finalized.");
    }
};

SANDBOX_REGISTRATION {
    SANDBOX_REGISTER_GLOBAL_FUNCTION(&my_global_function, "my_global_function")
    SANDBOX_REGISTER_STATIC_FUNCTION(math_utility, &math_utility::multiply, "math_multiply")
    SANDBOX_REGISTER_CLASS(test_extension, "test_extension")
}



int main()
{
    SANDBOX_LOG_INFO("=== Starting Engine Tests ===");

    sandbox::properties config;
    config.set({"logger", "name"}, std::string("engine_core"));
    config.set({"logger", "level"}, std::string("trace"));
    config.set({"window", "resolution", "width"}, 1920);
    config.set({"window", "resolution", "height"}, 1080);

    sandbox::engine app;
    app.initialize(config);

    SANDBOX_LOG_INFO("=== Testing Registry ===");

    sandbox::registry::invoke("my_global_function", nullptr);

    rttr::variant math_result = sandbox::registry::invoke_static("math_utility", "math_multiply", 5, 4);
    if (math_result.is_valid() && math_result.is_type<int>())
    {
        SANDBOX_LOG_INFO("Registry: Static math_multiply(5, 4) returned {}", math_result.get_value<int>());
    }

    SANDBOX_LOG_INFO("=== Testing Engine Structure ===");

    app.create_stage("physics_stage");

    app.create_object<position>("player_1", 0.0f, 0.0f);
    app.create_object<position>("player_2", 10.0f, 20.0f);

    app.create_system<position>(
        "movement_system",
        "physics_stage",
        [](position& pos) {
            pos.x += 0.1f;
            pos.y += 0.1f;
        }
    );

    SANDBOX_LOG_INFO("=== Loading Extension ===");

    app.create_extension("core_modules", "test_extension");

    SANDBOX_LOG_INFO("=== Entering Main Loop (Press Ctrl+C to exit) ===");

    int frame_count = 0;
    while (true)
    {
        app.progress();

        if (frame_count % 60 == 0)
        {
            auto* p1 = app.get_object<position>("player_1");
            if (p1)
            {
                SANDBOX_LOG_DEBUG("Player 1 Position: x={:.2f}, y={:.2f}", p1->x, p1->y);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        frame_count++;
    }

    return 0;
}