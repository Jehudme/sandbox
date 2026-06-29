#pragma once
// No need to include handle.h here, as the .inl is strictly included BY handle.h

namespace sandbox::sdk {

    template <typename T>
    inline sandbox_handle_t pack(T* ptr) noexcept {
        return sandbox_handle_t{ reinterpret_cast<uintptr_t>(ptr) };
    }

    template <typename T>
    inline T* unpack(sandbox_handle_t handle) noexcept {
        return reinterpret_cast<T*>(handle.token);
    }

} // namespace sandbox::sdk