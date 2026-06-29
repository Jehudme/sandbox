// suite/src/suite_full_boot_pipeline.cpp
// Integration: Full engine boot pipeline with service auto-resolution.

#include <catch2/catch_all.hpp>
#include "../../../sandbox/include/sandbox/abi/bootstrapper.h"
#include "core/bootstrapper.h"

#include <flecs.h>
#include <vector>
#include <string>
#include <algorithm>

using sandbox::core::bootstrapper_t;
using sandbox::core::module_info_t;
using sandbox::core::service_info_t;

// ---------------------------------------------------------------------------
// Services
// ---------------------------------------------------------------------------
struct IRenderer { int frame_count; };
struct IAudio    { float master_volume; };
struct IInput    { bool is_pressed; };

static IRenderer global_renderer_singleton = { .frame_count = 0 };
static IAudio    global_audio_singleton    = { .master_volume = 1.0f };
static IInput    global_input_singleton    = { .is_pressed = false };

SANDBOX_DECLARE_SERVICE(RendererService, IRenderer)
SANDBOX_DEFINE_SERVICE(RendererService, IRenderer, &global_renderer_singleton, {
    .name = "IRenderer", .description = "Renderer interface",
    .architecture = "sandbox::system", .version_major = 1, .version_minor = 0, .init_fn = nullptr,
})

SANDBOX_DECLARE_SERVICE(AudioService, IAudio)
SANDBOX_DEFINE_SERVICE(AudioService, IAudio, &global_audio_singleton, {
    .name = "IAudio", .description = "Audio interface",
    .architecture = "sandbox::system", .version_major = 1, .version_minor = 0, .init_fn = nullptr,
})

SANDBOX_DECLARE_SERVICE(InputService, IInput)
SANDBOX_DEFINE_SERVICE(InputService, IInput, &global_input_singleton, {
    .name = "IInput", .description = "Input interface",
    .architecture = "sandbox::system", .version_major = 1, .version_minor = 0, .init_fn = nullptr,
})

// ---------------------------------------------------------------------------
// Modules
// ---------------------------------------------------------------------------
struct OpenGLRendererModule { OpenGLRendererModule(ecs_world_t*) {} };
struct FMODAudioModule      { FMODAudioModule(ecs_world_t*)      {} };
struct RawInputModule       { RawInputModule(ecs_world_t*)       {} };
struct GameLayerModule      { GameLayerModule(ecs_world_t*)      {} };

SANDBOX_DECLARE_MODULE(OpenGLRendererModule, {
    .name = "OpenGLRendererModule", .description = "OpenGL renderer",
    .architecture = "sandbox::system", .version_major = 1, .version_minor = 0, .version_patch = 5,
    .service = &RendererService_info, .requirements = nullptr, .requirement_count = 0, .init_fn = nullptr,
})

SANDBOX_DECLARE_MODULE(FMODAudioModule, {
    .name = "FMODAudioModule", .description = "FMOD audio",
    .architecture = "sandbox::system", .version_major = 1, .version_minor = 0, .version_patch = 2,
    .service = &AudioService_info, .requirements = nullptr, .requirement_count = 0, .init_fn = nullptr,
})

SANDBOX_DECLARE_MODULE(RawInputModule, {
    .name = "RawInputModule", .description = "Raw OS input",
    .architecture = "sandbox::system", .version_major = 1, .version_minor = 0, .version_patch = 0,
    .service = &InputService_info, .requirements = nullptr, .requirement_count = 0, .init_fn = nullptr,
})

static sandbox_requirement_info_t game_layer_requirements[] = {
    { SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
      "IRenderer", "sandbox::system", 1, 0, -1 },
    { SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
      "IAudio", "sandbox::system", 1, 0, -1 },
    { SANDBOX_REQUIREMENT_KIND_SERVICE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
      "IInput", "sandbox::system", 1, 0, -1 },
};

SANDBOX_DECLARE_MODULE(GameLayerModule, {
    .name = "GameLayerModule", .description = "High-level game layer",
    .architecture = "sandbox::system", .version_major = 1, .version_minor = 0, .version_patch = 0,
    .service = nullptr, .requirements = game_layer_requirements, .requirement_count = 3, .init_fn = nullptr,
})

// Re-stage helper (sandbox may have been cleared by another test's reset)
static void restage_all() {
    sandbox_stage_module(&OpenGLRendererModule_info);
    sandbox_stage_module(&FMODAudioModule_info);
    sandbox_stage_module(&RawInputModule_info);
    sandbox_stage_module(&GameLayerModule_info);
}

// ---------------------------------------------------------------------------
// Suite: Full boot pipeline
// ---------------------------------------------------------------------------
TEST_CASE("Suite: Boot via top-level module auto-pulls providers", "[suite][full_boot]")
{
    restage_all();

    bootstrapper_t b;
    b.activate("sandbox::system", "GameLayerModule", 1, 0, 0);

    flecs::world w;
    REQUIRE_NOTHROW(b.boot(w));

    SECTION("all three services are present in Flecs world") {
        REQUIRE(SANDBOX_GET_SERVICE(w, RendererService) != nullptr);
        REQUIRE(SANDBOX_GET_SERVICE(w, AudioService)    != nullptr);
        REQUIRE(SANDBOX_GET_SERVICE(w, InputService)    != nullptr);
    }

    SECTION("renderer sdk pointer is correct") {
        const RendererService* r = SANDBOX_GET_SERVICE(w, RendererService);
        REQUIRE(r != nullptr);
        REQUIRE(r->api == &global_renderer_singleton);
    }

    SECTION("audio sdk pointer is correct") {
        const AudioService* a = SANDBOX_GET_SERVICE(w, AudioService);
        REQUIRE(a != nullptr);
        REQUIRE(a->api == &global_audio_singleton);
    }

    bootstrapper_t::reset();
}

TEST_CASE("Suite: Explicit activate of all providers is valid", "[suite][full_boot]")
{
    restage_all();

    bootstrapper_t b;
    b.activate("sandbox::system", "OpenGLRendererModule", 1, 0, -1);
    b.activate("sandbox::system", "FMODAudioModule",      1, 0, -1);
    b.activate("sandbox::system", "RawInputModule",       1, 0, -1);
    b.activate("sandbox::system", "GameLayerModule",      1, 0,  0);

    flecs::world w;
    REQUIRE_NOTHROW(b.boot(w));

    SECTION("all three services reachable after explicit activation") {
        REQUIRE(SANDBOX_GET_SERVICE(w, RendererService) != nullptr);
        REQUIRE(SANDBOX_GET_SERVICE(w, AudioService)    != nullptr);
        REQUIRE(SANDBOX_GET_SERVICE(w, InputService)    != nullptr);
    }

    bootstrapper_t::reset();
}
