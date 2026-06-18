#include "sandbox/core/bootstrapper.h"
#include "core/bootstrapper.h"

void sandbox_stage_service(const sandbox_service_info_t* info) {
    if (info) {
        sandbox::core::bootstrapper_t::stage_service(*info);
    }
}

void sandbox_stage_module(const sandbox_module_info_t* info) {
    if (info) {
        sandbox::core::bootstrapper_t::stage_module(*info);
    }
}

void sandbox_index_library(const char* library_path) {
    if (library_path) {
        sandbox::core::bootstrapper_t::index_library(library_path);
    }
}
