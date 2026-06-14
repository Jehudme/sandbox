#include "sandbox/core/bootstrapper.h"
#include "core/bootstrapper.h"

bool sandbox_stage_service(const sandbox_service_info_t& info) {
    sandbox::core::Bootstrapper::stage_service(info);
    return true;
}

bool sandbox_stage_module(const sandbox_module_info_t &info) {
    sandbox::core::Bootstrapper::stage_module(info);
    return true;
}
