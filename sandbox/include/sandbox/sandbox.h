#pragma once

/// SANDBOX ENGINE MASTER INCLUDE FILE
/// Includes the entire public API for the Sandbox Meta-Engine.

#include "sandbox/core/platform.h"
#include "sandbox/core/ecs.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"

// Unified event payloads, controls, exceptions, and macros
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/event_bus/plugin_events.h"
#include "sandbox/event_bus/runner_events.h"
#include "sandbox/event_bus/filesystem_events.h"

#include "sandbox/event_bus/event_bus.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/utilities/properties.h"