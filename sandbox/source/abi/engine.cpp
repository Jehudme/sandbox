#include "sandbox/abi/engine.h"
#include "core/engine.h"
#include <iostream>

extern "C" {

sandbox_engine_t *sandbox_engine_create(void) {
  auto *internal_engine = new sandbox::core::engine_t();
  return reinterpret_cast<sandbox_engine_t *>(internal_engine);
}

void sandbox_engine_destroy(sandbox_engine_t *engine) {
  if (engine) {
    delete reinterpret_cast<sandbox::core::engine_t *>(engine);
  }
}

bool sandbox_engine_initialize(sandbox_engine_t *engine,
                               sandbox_properties_handle_t properties) {
  if (!engine || (!SANDBOX_HANDLE_IS_VALID(properties)))
    return false;

  auto *internal_engine = reinterpret_cast<sandbox::core::engine_t *>(engine);
  auto *internal_properties = reinterpret_cast<sandbox::core::properties_t *>(properties.token);

  try {
    internal_engine->initialize(*internal_properties);
    return true;
  } catch (const std::exception &ex) {
    std::cerr << "[Engine] Initialization failed: " << ex.what() << "\n";
    return false;
  } catch (...) {
    std::cerr << "[Engine] Initialization failed with an unknown error.\n";
    return false;
  }
}

ecs_world_t *sandbox_engine_get_ecs(sandbox_engine_t *engine) {
  if (!engine)
    return nullptr;
  auto *internal_engine = reinterpret_cast<sandbox::core::engine_t *>(engine);
  return reinterpret_cast<ecs_world_t *>(internal_engine->entity_world.c_ptr());
}
}
