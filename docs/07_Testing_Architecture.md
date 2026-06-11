# 7.  Testing the Architecture

The Sandbox Engine maintains a rigorous test suite using **Catch2**.

## Mocking the C-ABI Boundary
Because the engine interacts with plugins via C-ABI structures (`logger_service`, `filesystem_service`), the unit tests directly construct these structs using function pointers to mock behavior.

For example, `sandbox_unit_tests` tests the `sdk_wrappers` (like `sandbox::sdk::filesystem`) by passing them a mocked C struct, verifying that errors cross the DLL boundary smoothly as integers and translate correctly into `std::expected` types on the C++ wrapper side.

## `libtest` Integration
The engine contains a physical dynamic library plugin compiled specifically for testing (`test_lib_mock.so`).
During integration tests (`sandbox_tests`), the engine actually mounts a VFS environment, loads `manifest.json`, dynamically discovers `test_lib_mock.so`, and attempts to invoke its `SandboxLibraryMain` entry point.

This guarantees that:
1.  Dynamic symbols are properly exported via `SANDBOX_EXPORT`.
2.  The plugin bootstrapper successfully resolves metadata using Kahn's topological sort.
3.  The ECS successfully passes context across the DLL wall.

## Graph Safety Validation
The unit tests simulate toxic plugin loads to ensure stability. They mock dependencies representing:
*   **Missing Libraries:** A manifest requests a library that isn't on disk.
*   **Version Conflicts:** Plugin A requires Logger v1.0, but Plugin B requires Logger v2.0.
*   **Cyclic Dependencies:** Plugin A -> Plugin B -> Plugin A.

The tests ensure that the `bootstrapper` algorithm catches all these scenarios and cleanly halts execution with `sandbox::boot_error` exceptions before any native memory allocation or ECS execution begins.
