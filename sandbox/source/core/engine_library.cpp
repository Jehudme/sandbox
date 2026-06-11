#include "sandbox/core/plugin.h"
#include "modules/logger/logger.h"
#include "modules/filesystem/filesystem.h"
#include "modules/runner/runner.h"

using sandbox::modules::logger;
using sandbox::modules::filesystem_module;
using sandbox::modules::runner;

// Treat the engine's core infrastructure as a standard Sandbox Plugin Library
SANDBOX_DECLARE_SERVICE(logger_service, 1, 0);
SANDBOX_DECLARE_MODULE(logger, core_logger, 1, 0, 0, "logger_service");

SANDBOX_DECLARE_SERVICE(filesystem_service, 1, 0);
SANDBOX_DECLARE_MODULE(filesystem_module, core_vfs, 1, 0, 0, "filesystem_service");

SANDBOX_DECLARE_SERVICE(runner_service, 1, 0);
SANDBOX_DECLARE_MODULE(runner, core_runner, 1, 0, 0, "runner_service");
