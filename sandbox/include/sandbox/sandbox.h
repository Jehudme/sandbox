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
// 2. Events, Payloads, Controls & Exceptions (unified headers)
// ----------------------------------------------------------------------------
#include "sandbox/events/logger.h"
#include "sandbox/events/plugins.h"
#include "sandbox/events/runner.h"      // Also contains runner_controls + SANDBOX_RUNNER_* macros
#include "sandbox/events/filesystem.h"  // Also contains filesystem_controls + SANDBOX_FS_* macros + exception classes

// ----------------------------------------------------------------------------
// 3. Plugin Loader Macros
// ----------------------------------------------------------------------------
#include "sandbox/macros/logger.h"
#include "sandbox/macros/plugins.h"

// ----------------------------------------------------------------------------
// 4. Utilities & Helpers
// ----------------------------------------------------------------------------
#include "sandbox/utilities/events.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/utilities/properties.h"