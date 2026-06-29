#pragma once

#include <stdbool.h>
#include <flecs.h>

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t sandbox_requirement_kind_t;
enum {
    SANDBOX_REQUIREMENT_KIND_SERVICE = 0,
    SANDBOX_REQUIREMENT_KIND_MODULE
};

typedef uint32_t sandbox_requirement_strictness_t;
enum {
    SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED = 0,
    SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED
};

typedef struct {
    sandbox_requirement_kind_t kind;
    sandbox_requirement_strictness_t strictness;
    const char* name;
    const char* architecture;
    int32_t version_major;
    int32_t version_minor;
    int32_t version_patch; /* only for module, -1 to set to the highest patch available. */
} sandbox_requirement_info_t;

typedef struct {
    uint32_t struct_size;
    const char* name;
    const char* description;
    const char* architecture;
    int32_t version_major;
    int32_t version_minor;
    void (*init_fn)(ecs_world_t* ecs);
} sandbox_service_info_t;

typedef struct {
    uint32_t struct_size;
    const char* name;
    const char* description;
    const char* architecture;
    int32_t version_major;
    int32_t version_minor;
    int32_t version_patch;

    /* Linked service (Optional, NULL if no service provided) */
    const sandbox_service_info_t* service;

    const sandbox_requirement_info_t* requirements;
    size_t requirement_count;

    void (*init_fn)(ecs_world_t* ecs);
} sandbox_module_info_t;

/* engine_t hooks (Implemented in engine sdk, called automatically by plugins) */
SANDBOX_API bool sandbox_stage_service(const sandbox_service_info_t* info);
SANDBOX_API bool sandbox_stage_module(const sandbox_module_info_t* info);

/* Indexing hooks */
SANDBOX_API void sandbox_index_library(const char* library_path);

/* ========================================================================== */
/* INSTANCE API (Access to the engine's bootstrapper)                         */
/* ========================================================================== */

/* Opaque pointer for the bootstrapper instance */
typedef struct sandbox_bootstrapper sandbox_bootstrapper_t;

/* ECS Component for holding the bootstrapper */
typedef struct {
    sandbox_bootstrapper_t* internal_bootstrapper;
} sandbox_bootstrapper_component_t;

extern ECS_COMPONENT_DECLARE(sandbox_bootstrapper_component_t);

SANDBOX_API sandbox_bootstrapper_t* sandbox_get_bootstrapper(ecs_world_t* ecs);

SANDBOX_API bool sandbox_bootstrapper_activate(sandbox_bootstrapper_t* bootstrapper, const char* architecture, const char* name, int version_major, int version_minor, int version_patch);
SANDBOX_API bool sandbox_bootstrapper_activate_string(sandbox_bootstrapper_t* bootstrapper, const char* module_str);
SANDBOX_API bool sandbox_bootstrapper_boot(sandbox_bootstrapper_t* bootstrapper, ecs_world_t* ecs);

/* ========================================================================== */
/* SERVICE DECLARATION (100% Pure C ABI Safe)                                 */
/* ========================================================================== */

/**
 * SANDBOX_DECLARE_SERVICE
 * Generates the struct, Flecs Component, auto-init function, and stages it.
 */
#define SANDBOX_DECLARE_SERVICE(ServiceClass, IModuleType, ...) \
    typedef struct ServiceClass { \
        IModuleType* api; \
        const sandbox_service_info_t* info; \
    } ServiceClass; \
    \
    extern ECS_COMPONENT_DECLARE(ServiceClass); \
    static const sandbox_service_info_t ServiceClass##_info_decl = __VA_ARGS__;

#define SANDBOX_DEFINE_SERVICE(ServiceClass, IModuleType, api_ptr) \
    ECS_COMPONENT_DECLARE(ServiceClass); \
    static sandbox_service_info_t ServiceClass##_info = ServiceClass##_info_decl; \
    \
    static void ServiceClass##_init_fn(ecs_world_t* ecs) { \
        ECS_COMPONENT_DEFINE(ecs, ServiceClass); \
        ServiceClass inst; \
        inst.api = (api_ptr); \
        inst.info = &ServiceClass##_info; \
        ecs_set_id(ecs, ecs_id(ServiceClass), ecs_id(ServiceClass), sizeof(ServiceClass), &inst); \
    } \
    \
    SANDBOX_CONSTRUCTOR(__sandbox_stage_##ServiceClass) { \
        ServiceClass##_info.struct_size = sizeof(sandbox_service_info_t); \
        ServiceClass##_info.init_fn = ServiceClass##_init_fn; \
        sandbox_stage_service(&ServiceClass##_info); \
    }

#ifdef __cplusplus
} /* End extern "C" */
#endif

/* ========================================================================== */
/* MODULE DECLARATION (Seamless C and C++ Support)                            */
/* ========================================================================== */

/* Internal helper: Chooses correct Flecs import syntax based on language */
#ifdef __cplusplus
    #define __SANDBOX_IMPORT_MODULE(ecs, ModuleClass) \
        do { flecs::world __w(ecs); __w.import<ModuleClass>(); } while(0)
#else
    #define __SANDBOX_IMPORT_MODULE(ecs, ModuleClass) \
        ECS_IMPORT(ecs, ModuleClass)
#endif

/**
 * SANDBOX_DECLARE_MODULE
 * Generates info struct, ties the service init via pointer, imports module, and stages it.
 */
#define SANDBOX_DECLARE_MODULE(ModuleClass, ...) \
    static sandbox_module_info_t ModuleClass##_info = __VA_ARGS__; \
    \
    static void ModuleClass##_init_fn(ecs_world_t* ecs) { \
        /* Safely check if a service is attached and initialize it first */ \
        if (ModuleClass##_info.service && ModuleClass##_info.service->init_fn) { \
            ModuleClass##_info.service->init_fn(ecs); \
        } \
        /* Import the module systems/components into Flecs */ \
        __SANDBOX_IMPORT_MODULE(ecs, ModuleClass); \
    } \
    \
    SANDBOX_CONSTRUCTOR(__sandbox_stage_##ModuleClass) { \
        ModuleClass##_info.struct_size = sizeof(sandbox_module_info_t); \
        ModuleClass##_info.init_fn = ModuleClass##_init_fn; \
        sandbox_stage_module(&ModuleClass##_info); \
    }

/* ========================================================================== */
/* 5. UNIFIED FETCH MACRO                                                     */
/* ========================================================================== */

#ifdef __cplusplus
    #define SANDBOX_GET_SERVICE(world_obj, ServiceClass) \
        (world_obj).try_get<ServiceClass>()
#else
    #define SANDBOX_GET_SERVICE(ecs_ptr, ServiceClass) \
        ecs_singleton_get((ecs_ptr), ServiceClass)
#endif