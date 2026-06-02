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
#include "sandbox/core/arguments.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"

// ----------------------------------------------------------------------------
// 2. Events & Payloads
// ----------------------------------------------------------------------------
#include "sandbox/events/logger.h"
#include "sandbox/events/plugins.h"
#include "sandbox/events/runner.h"
#include "sandbox/events/vfs.h"

// ----------------------------------------------------------------------------
// 3. Exceptions
// ----------------------------------------------------------------------------
#include "sandbox/exceptions/plugins.h"
#include "sandbox/exceptions/vfs_exceptions.h"

// ----------------------------------------------------------------------------
// 4. Utilities & Helpers
// ----------------------------------------------------------------------------
#include "sandbox/utilities/events.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/utilities/properties.h"

// ----------------------------------------------------------------------------
// 5. Public Macros
// ----------------------------------------------------------------------------
#include "sandbox/macros/logger.h"
#include "sandbox/macros/plugins.h"
#include "sandbox/macros/runner.h"
#include "sandbox/macros/vfs.h"