#pragma once
#include "sandbox/abi/handle.h"

namespace sandbox::sdk {

    /**
     * @brief Safely packs a C++ pointer into a strictly typed ABI handle.
     * @tparam HandleType The ABI struct type (e.g., sandbox_audio_handle_t)
     * @tparam InternalType The hidden C++ struct type
     */
    template <typename HandleType, typename InternalType>
    [[nodiscard]] inline HandleType pack(InternalType* ptr) noexcept {
        return HandleType{ reinterpret_cast<uintptr_t>(ptr) };
    }

    /**
     * @brief Safely unpacks a strictly typed ABI handle back into a C++ pointer.
     * @tparam InternalType The hidden C++ struct type
     * @tparam HandleType The ABI struct type (e.g., sandbox_audio_handle_t)
     */
    template <typename InternalType, typename HandleType>
    [[nodiscard]] inline InternalType* unpack(HandleType handle) noexcept {
        return reinterpret_cast<InternalType*>(handle.token);
    }

} // namespace sandbox::sdk