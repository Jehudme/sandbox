#pragma once
#include "core/bootstrapper.h"
#include <algorithm>
#include <string>

struct bootstrapper_test_accessor {
    static void reset() {
        auto is_test_arch = [](const char* a) {
            if (!a) return true;
            std::string arch = a;
            return arch != "sandbox" && arch != "sandbox::core";
        };

        sandbox::core::bootstrapper_t::m_modules.erase(
            std::remove_if(sandbox::core::bootstrapper_t::m_modules.begin(), sandbox::core::bootstrapper_t::m_modules.end(),
                [&](const sandbox::core::module_info_t& m) { return is_test_arch(m.architecture); }),
            sandbox::core::bootstrapper_t::m_modules.end());

        sandbox::core::bootstrapper_t::m_services.erase(
            std::remove_if(sandbox::core::bootstrapper_t::m_services.begin(), sandbox::core::bootstrapper_t::m_services.end(),
                [&](const sandbox::core::service_info_t& s) { return is_test_arch(s.architecture); }),
            sandbox::core::bootstrapper_t::m_services.end());

        sandbox::core::bootstrapper_t::m_loader = sandbox::core::library_loader_t();
    }
};
