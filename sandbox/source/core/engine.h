#pragma once
#include "properties.h"
#include <flecs.h>
#include <memory>

namespace sandbox::core {
class bootstrapper_t;

/**
 * @brief The core engine class responsible for initializing and booting the
 * sandbox environment.
 */
class engine_t {
public:
  /**
   * @brief Constructs a new engine instance.
   */
  engine_t();

  /**
   * @brief Destroys the engine instance.
   */
  ~engine_t();

  /**
   * @brief Initializes the engine with the provided properties.
   * @param properties The properties to configure the engine.
   */
  void initialize(properties_t &properties);

public:
  flecs::world entity_world;

private:
  /**
   * @brief Saves the bootstrapper into the entity world for plugins to access.
   */
  void register_bootstrapper();

  /**
   * @brief Boots the engine by indexing libraries and activating plugins.
   */
  void boot();

private:
  std::unique_ptr<properties_t> m_arguments;
  std::unique_ptr<bootstrapper_t> m_bootstrapper;
};
} // namespace sandbox::core
