#include <iostream>
#include <flecs.h>
#include "sandbox/core/plugin.h"

// ---------------------------------------------------------
// 1. A basic module that gets imported by the master library
// ---------------------------------------------------------
struct QuickTestModule {
    explicit QuickTestModule(flecs::world& world) {
        world.module<QuickTestModule>();
        std::cout << "  -> [QuickTest] Module successfully loaded into memory!\n";
    }
};
SANDBOX_DEFINE_MODULE(QuickTestModule, QuickTest)

// ---------------------------------------------------------
// 2. The Master Entry point the Engine looks for
// ---------------------------------------------------------
struct QuickTestMasterLibrary {
    explicit QuickTestMasterLibrary(flecs::world& world) {
        world.module<QuickTestMasterLibrary>();
        
        std::cout << "\n[QuickTest] OS Linker successfully invoked the Master Library!\n";
        
        world.import<QuickTestModule>();
        
        std::cout << "[QuickTest] Initialization complete.\n\n";
    }
};

// Exposes the "SandboxLibraryMain" symbol securely
SANDBOX_DEFINE_LIBRARY(QuickTestMasterLibrary)