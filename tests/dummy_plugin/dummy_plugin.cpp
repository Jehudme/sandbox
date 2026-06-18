// tests/dummy_plugin/dummy_plugin.cpp
// Minimal shared library used exclusively by library_loader tests.
// It exports one symbol so the linker doesn't strip the entire object.

#if defined(_WIN32) || defined(_WIN64)
#define DUMMY_EXPORT __declspec(dllexport)
#else
#define DUMMY_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {

DUMMY_EXPORT int sandbox_dummy_version() {
    return 1;
}

} // extern "C"
