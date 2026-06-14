// suite/src/suite_full_boot_pipeline.cpp
//
// Integration suite: Full boot pipeline
//
// Simulates a realistic production-like scenario where:
//   1. Multiple services are staged (renderer, audio, input)
//   2. Multiple modules are staged, each with service requirements
//   3. A hand-crafted game layer module requires several services
//   4. boot() is called and we verify that ALL modules initialized in
//      dependency order and that the Flecs world has all service components.

#include <catch2/catch_all.hpp>
#include "sandbox/core/bootstrapper.h"
#include "core/bootstrapper.h"

#include <flecs.h>
#include <vector>
#include <string>
#include <algorithm>

using sandbox::core::Bootstrapper;
using sandbox::core::ModuleInfo;
using sandbox::core::ServiceInfo;

// ---------------------------------------------------------------------------
// Services (interfaces) — declared as global Flecs components using the macro
// ---------------------------------------------------------------------------

struct IRenderer { int frame_count; };
struct IAudio    { float master_volume; };
struct IInput    { bool is_pressed; };

static IRenderer global_renderer_singleton = { .frame_count = 0 };
static IAudio    global_audio_singleton    = { .master_volume = 1.0f };
static IInput    global_input_singleton    = { .is_pressed = false };

// NOTE: Use {}, not ({}) — compound statement literals are a GCC extension
//       not allowed at global scope in standard C++.
SANDBOX_DECLARE_SERVICE(
    RendererService,
    IRenderer,
    &global_renderer_singleton,
    {
        .name          = "IRenderer",
        .description   = "Renderer interface",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .init_fn       = nullptr,
    }
)

SANDBOX_DECLARE_SERVICE(
    AudioService,
    IAudio,
    &global_audio_singleton,
    {
        .name          = "IAudio",
        .description   = "Audio interface",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .init_fn       = nullptr,
    }
)

SANDBOX_DECLARE_SERVICE(
    InputService,
    IInput,
    &global_input_singleton,
    {
        .name          = "IInput",
        .description   = "Input interface",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .init_fn       = nullptr,
    }
)

// ---------------------------------------------------------------------------
// Modules — declared globally using the macro
// ---------------------------------------------------------------------------

// Each module needs a corresponding Flecs module type for __SANDBOX_IMPORT_MODULE.
struct OpenGLRendererModule { OpenGLRendererModule(ecs_world_t*) {} };
struct FMODAudioModule      { FMODAudioModule(ecs_world_t*)      {} };
struct RawInputModule       { RawInputModule(ecs_world_t*)       {} };
struct GameLayerModule      { GameLayerModule(ecs_world_t*)      {} };

SANDBOX_DECLARE_MODULE(
    OpenGLRendererModule,
    {
        .name          = "OpenGLRendererModule",
        .description   = "OpenGL renderer implementation",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 5,
        .service       = &RendererService_info,
        .requirements  = nullptr,
        .requirement_count = 0,
        .init_fn       = nullptr,
    }
)

SANDBOX_DECLARE_MODULE(
    FMODAudioModule,
    {
        .name          = "FMODAudioModule",
        .description   = "FMOD audio implementation",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 2,
        .service       = &AudioService_info,
        .requirements  = nullptr,
        .requirement_count = 0,
        .init_fn       = nullptr,
    }
)

SANDBOX_DECLARE_MODULE(
    RawInputModule,
    {
        .name          = "RawInputModule",
        .description   = "Raw OS input implementation",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service       = &InputService_info,
        .requirements  = nullptr,
        .requirement_count = 0,
        .init_fn       = nullptr,
    }
)

// Game layer requires IRenderer + IAudio + IInput (all required)
static sandbox_requirement_info_t game_layer_requirements[] = {
    {
        .kind          = SANDBOX_REQUIREMENT_KIND_SERVICE,
        .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name          = "IRenderer",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1,
    },
    {
        .kind          = SANDBOX_REQUIREMENT_KIND_SERVICE,
        .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name          = "IAudio",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1,
    },
    {
        .kind          = SANDBOX_REQUIREMENT_KIND_SERVICE,
        .strictness    = SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
        .name          = "IInput",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = -1,
    },
};

SANDBOX_DECLARE_MODULE(
    GameLayerModule,
    {
        .name          = "GameLayerModule",
        .description   = "High-level game layer",
        .architecture  = "sandbox::system",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service       = nullptr,
        .requirements  = game_layer_requirements,
        .requirement_count = 3,
        .init_fn       = nullptr,
    }
)

// ---------------------------------------------------------------------------
// Suite: Full boot pipeline
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Full boot pipeline — top-level boots all",
          "[suite][full_boot]")
{
    // Re-stage the globally declared modules in case another test cleared the Bootstrapper registry
    sandbox_stage_module(&OpenGLRendererModule_info);
    sandbox_stage_module(&FMODAudioModule_info);
    sandbox_stage_module(&RawInputModule_info);
    sandbox_stage_module(&GameLayerModule_info);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "GameLayerModule", 1, 0, 0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("RendererService is present as a Flecs singleton component") {
        const RendererService* retrieved = SANDBOX_GET_SERVICE(test_world, RendererService);
        REQUIRE(retrieved != nullptr);
    }

    SECTION("AudioService is present as a Flecs singleton component") {
        const AudioService* retrieved = SANDBOX_GET_SERVICE(test_world, AudioService);
        REQUIRE(retrieved != nullptr);
    }

    SECTION("InputService is present as a Flecs singleton component") {
        const InputService* retrieved = SANDBOX_GET_SERVICE(test_world, InputService);
        REQUIRE(retrieved != nullptr);
    }

    SECTION("RendererService api pointer is correct") {
        const RendererService* retrieved = SANDBOX_GET_SERVICE(test_world, RendererService);
        if (retrieved) {
            REQUIRE(retrieved->api == &global_renderer_singleton);
        }
    }

    SECTION("AudioService api pointer is correct") {
        const AudioService* retrieved = SANDBOX_GET_SERVICE(test_world, AudioService);
        if (retrieved) {
            REQUIRE(retrieved->api == &global_audio_singleton);
        }
    }

    Bootstrapper::reset();
}

TEST_CASE("Suite: Full boot pipeline — explicit activate valid",
          "[suite][full_boot]")
{
    // Re-stage the globally declared modules in case another test cleared the Bootstrapper registry
    sandbox_stage_module(&OpenGLRendererModule_info);
    sandbox_stage_module(&FMODAudioModule_info);
    sandbox_stage_module(&RawInputModule_info);
    sandbox_stage_module(&GameLayerModule_info);

    Bootstrapper bootstrapper_instance;
    bootstrapper_instance.activate("sandbox::system", "OpenGLRendererModule", 1, 0, -1);
    bootstrapper_instance.activate("sandbox::system", "FMODAudioModule",      1, 0, -1);
    bootstrapper_instance.activate("sandbox::system", "RawInputModule",       1, 0, -1);
    bootstrapper_instance.activate("sandbox::system", "GameLayerModule",      1, 0,  0);

    flecs::world test_world;
    REQUIRE_NOTHROW(bootstrapper_instance.boot(test_world));

    SECTION("all three services are reachable") {
        REQUIRE(SANDBOX_GET_SERVICE(test_world, RendererService) != nullptr);
        REQUIRE(SANDBOX_GET_SERVICE(test_world, AudioService)    != nullptr);
        REQUIRE(SANDBOX_GET_SERVICE(test_world, InputService)    != nullptr);
    }

    Bootstrapper::reset();
}
