#pragma once

/**
 * Sandbox Meta-Engine Master Header
 * Version: 0.1.0
 */

// --- CORE ---
#include "sandbox/core/engine.h"
#include "sandbox/core/extension.h"
#include "sandbox/core/properties.h"

// --- ECS (Automated Flecs Wrappers) ---
#include "sandbox/ecs/systems.h"
#include "sandbox/ecs/triggers.h"
#include "sandbox/ecs/events.h"
#include "sandbox/ecs/stages.h"
#include "sandbox/ecs/scopes.h"

// --- DATA (Memory & Metadata) ---
#include "sandbox/data/caches.h"
#include "sandbox/data/storage.h"
#include "sandbox/data/registry.h"
#include "sandbox/data/registration.h"

// --- IO (Files & Serialization) ---
#include "sandbox/io/filesystem.h"
#include "sandbox/io/serializer.h"

// --- SYSTEM (Engine Services) ---
#include "sandbox/system/clock.h"
#include "sandbox/system/dependencies.h"

// --- DIAGNOSTICS ---
#include "sandbox/diagnostics/logger.h"
#include "sandbox/diagnostics/scoped_logger.h"
#include "sandbox/diagnostics/diagnostics.h"
