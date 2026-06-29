#pragma once

#include "sandbox/abi/handle.h"

namespace sandbox::sdk {

    /**
     * @brief Safely packs any C++ pointer into an ABI handle token.
     */
    template <typename T>
    [[nodiscard]] sandbox_handle_t pack(T* ptr) noexcept;

    /**
     * @brief Safely unpacks an ABI handle token back into a strongly-typed C++ pointer.
     */
    template <typename T>
    [[nodiscard]] T* unpack(sandbox_handle_t handle) noexcept;

} // namespace sandbox::sdk

// Include the inline implementations at the bottom
#include "handle.inl"