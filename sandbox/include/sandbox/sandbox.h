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
#include "sandbox/ecs/systems_ext.h"
#include "sandbox/ecs/triggers_ext.h"
#include "sandbox/ecs/events_ext.h"
#include "sandbox/ecs/stages_ext.h"
#include "sandbox/ecs/scopes_ext.h"

// --- DATA (Memory & Metadata) ---
#include "sandbox/data/caches_ext.h"
#include "sandbox/data/storage_ext.h"
#include "sandbox/data/registry.h"
#include "sandbox/data/registration.h"

// --- IO (Files & Serialization) ---
#include "sandbox/io/filesystem_ext.h"
#include "sandbox/io/serializer_ext.h"

// --- SYSTEM (Engine Services) ---
#include "sandbox/system/clock_ext.h"
#include "sandbox/system/dependencies_ext.h"

// --- DIAGNOSTICS ---
#include "sandbox/diagnostics/logger.h"
#include "sandbox/diagnostics/scoped_logger.h"
#include "sandbox/diagnostics/diagnostics_ext.h"
