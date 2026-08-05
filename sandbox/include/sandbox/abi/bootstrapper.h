#pragma once

#include <stdbool.h>
#include <flecs.h>

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of requirement kinds.
 */
typedef uint32_t sandbox_requirement_kind_t;
enum {
    SANDBOX_REQUIREMENT_KIND_SERVICE = 0,
    SANDBOX_REQUIREMENT_KIND_MODULE
};

/**
 * @brief Enumeration of requirement strictness.
 */
typedef uint32_t sandbox_requirement_strictness_t;
enum {
    SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED = 0,
    SANDBOX_REQUIREMENT_STRICTNESS_EXPECTED
};

/**
 * @brief Enumeration of requirement kinds.
 */
typedef struct {
    sandbox_requirement_kind_t kind;
    sandbox_requirement_strictness_t strictness;
    const char* name;
    const char* architecture;
    int32_t version_major;
    int32_t version_minor;
    int32_t version_patch; /**
 * @brief only for module, -1 to set to the highest patch available.
 */
} sandbox_requirement_info_t;

/**
 * @brief Represents a service definition.
 */
typedef struct {
    uint32_t struct_size;
    const char* name;
    const char* description;
    const char* architecture;
    int32_t version_major;
    int32_t version_minor;
    void (*init_fn)(ecs_world_t* ecs);
} sandbox_service_info_t;

/**
 * @brief Represents a module or service requirement.
 */
typedef struct {
    uint32_t struct_size;
    const char* name;
    const char* description;
    const char* architecture;
    int32_t version_major;
    int32_t version_minor;
    int32_t version_patch;

        const sandbox_service_info_t* service;

    const sandbox_requirement_info_t* requirements;
    size_t requirement_count;

    void (*init_fn)(ecs_world_t* ecs);
} sandbox_module_info_t;

/**
 * @brief engine_t hooks (Implemented in engine sdk, called automatically by plugins)
 */
/**
 * @brief Stages a service for initialization.
 * @param info Service information.
 * @return True on success.
 */
SANDBOX_API bool sandbox_stage_service(const sandbox_service_info_t* info);
/**
 * @brief Stages a module for initialization.
 * @param info Module information.
 * @return True on success.
 */
SANDBOX_API bool sandbox_stage_module(const sandbox_module_info_t* info);

/**
 * @brief Indexes a dynamic library for plugins.
 * @param ecs The entity component system world.
 * @param library_path The path to the library.
 */
SANDBOX_API void sandbox_load_library(ecs_world_t* ecs, const char* library_path);


typedef struct sandbox_bootstrapper sandbox_bootstrapper_t;

typedef struct {
    sandbox_bootstrapper_t* internal_bootstrapper;
} sandbox_bootstrapper_component_t;

/**
 * @brief Declares the bootstrapper ECS component.
 */
extern ECS_COMPONENT_DECLARE(sandbox_bootstrapper_component_t);

/**
 * @brief Retrieves the bootstrapper instance from the ECS world.
 * @param ecs The entity component system world.
 * @return The bootstrapper instance.
 */
SANDBOX_API sandbox_bootstrapper_t* sandbox_get_bootstrapper(ecs_world_t* ecs);

/**
 * @brief Activates a module by explicit architecture, name, and version.
 * @param bootstrapper The bootstrapper instance.
 * @param ecs The entity component system world.
 * @param architecture The module architecture.
 * @param name The module name.
 * @param version_major The major version.
 * @param version_minor The minor version.
 * @param version_patch The patch version.
 * @return True on success.
 */
SANDBOX_API bool sandbox_bootstrapper_activate(sandbox_bootstrapper_t* bootstrapper, ecs_world_t* ecs, const char* architecture, const char* name, int version_major, int version_minor, int version_patch);
/**
 * @brief Activates a module by a string descriptor.
 * @param bootstrapper The bootstrapper instance.
 * @param ecs The entity component system world.
 * @param module_str The module descriptor string.
 * @return True on success.
 */
SANDBOX_API bool sandbox_bootstrapper_activate_string(sandbox_bootstrapper_t* bootstrapper, ecs_world_t* ecs, const char* module_str);
/**
 * @brief Boots all activated plugins.
 * @param bootstrapper The bootstrapper instance.
 * @param ecs The entity component system world.
 * @return True on success.
 */
SANDBOX_API bool sandbox_bootstrapper_boot(sandbox_bootstrapper_t* bootstrapper, ecs_world_t* ecs);


/**
 * @brief Generates the struct, Flecs Component, auto-init function, and stages it.
 * @param ServiceClass The name of the service struct.
 * @param IModuleType The API type name.
 * @param ... The service info initialization list.
 */
#ifdef SANDBOX_FFI_GENERATION
#define SANDBOX_DECLARE_SERVICE(ServiceClass, IModuleType, ...) \
    typedef struct ServiceClass { \
        IModuleType* api; \
        const sandbox_service_info_t* info; \
    } ServiceClass; \
    \
    SANDBOX_API extern ECS_COMPONENT_DECLARE(ServiceClass); \
    extern sandbox_service_info_t ServiceClass##_info;
#else
#define SANDBOX_DECLARE_SERVICE(ServiceClass, IModuleType, ...) \
    typedef struct ServiceClass { \
        IModuleType* api; \
        const sandbox_service_info_t* info; \
    } ServiceClass; \
    \
    SANDBOX_API extern ECS_COMPONENT_DECLARE(ServiceClass); \
    static const sandbox_service_info_t ServiceClass##_info_decl = __VA_ARGS__; \
    extern sandbox_service_info_t ServiceClass##_info;
#endif

/**
 * @brief Defines the initialization function for the service.
 * @param ServiceClass The name of the service struct.
 * @param IModuleType The API type name.
 * @param api_ptr The pointer to the API instance.
 */
#define SANDBOX_DEFINE_SERVICE(ServiceClass, IModuleType, api_ptr) \
    SANDBOX_API ECS_COMPONENT_DECLARE(ServiceClass); \
    sandbox_service_info_t ServiceClass##_info = ServiceClass##_info_decl; \
    \
    static void ServiceClass##_init_fn(ecs_world_t* ecs) { \
        ecs_id(ServiceClass) = 0; \
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
}
#endif


#ifdef __cplusplus
    #define __SANDBOX_IMPORT_MODULE(ecs, ModuleClass) \
        do { flecs::world __w(ecs); __w.import<ModuleClass>(); } while(0)
#else
    #define __SANDBOX_IMPORT_MODULE(ecs, ModuleClass) \
        ECS_IMPORT(ecs, ModuleClass)
#endif

/**
 * @brief Generates info struct, ties the service init via pointer, imports module, and stages it.
 * @param ModuleClass The name of the module class.
 * @param ... The module info initialization list.
 */
#ifdef SANDBOX_FFI_GENERATION
#define SANDBOX_DECLARE_MODULE(ModuleClass, ...)
#else
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
#endif

/**
 * @brief Unified macro to fetch a service instance from the ECS world.
 * @param world_obj The flecs world object (C++) or ecs_world_t pointer (C).
 * @param ServiceClass The class of the service to fetch.
 */

#ifdef __cplusplus
    #define SANDBOX_GET_SERVICE(world_obj, ServiceClass) \
        (world_obj).try_get<ServiceClass>()
#else
    #define SANDBOX_GET_SERVICE(ecs_ptr, ServiceClass) \
        ecs_singleton_get((ecs_ptr), ServiceClass)
#endif