#pragma once

// ============================================================================
// SANDBOX ENGINE MASTER INCLUDE FILE
// Includes the entire public API for the Sandbox Meta-Engine.
// ============================================================================

// ----------------------------------------------------------------------------
// 1. Core Architecture
// ----------------------------------------------------------------------------
#include "sandbox/core/platform.h"      // Always include platform first for API macros
#include "sandbox/core/ecs.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"

// ----------------------------------------------------------------------------
// 2. Event Bus (unified event payloads, controls, exceptions, and macros)
// ----------------------------------------------------------------------------
#include "sandbox/event_bus/logger_events.h"     // log struct + SANDBOX_INFO/WARN/ERROR etc.
#include "sandbox/event_bus/plugin_events.h"     // plugins::load_request + SANDBOX_PLUGIN_LOAD
#include "sandbox/event_bus/runner_events.h"     // runner events + SANDBOX_RUNNER_* macros
#include "sandbox/event_bus/filesystem_events.h" // filesystem events + SANDBOX_FS_* macros + exceptions

// ----------------------------------------------------------------------------
// 3. Utilities & Helpers
// ----------------------------------------------------------------------------
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/utilities/properties.h"