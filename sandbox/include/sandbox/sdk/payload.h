#pragma once
#include "sandbox/core/abi_types.h"
#include <string>
#include <stdexcept>

namespace sandbox::sdk {

    class payload {
    public:
        payload() = default;
        ~payload() {
            if (m_data.free_func && m_data.bytes) {
                m_data.free_func(m_data.bytes);
            }
        }

        // Delete copy
        payload(const payload&) = delete;
        payload& operator=(const payload&) = delete;

        // Allow move
        payload(payload&& other) noexcept : m_data(other.m_data) {
            other.m_data = {};
        }

        payload& operator=(payload&& other) noexcept {
            if (this != &other) {
                if (m_data.free_func && m_data.bytes) {
                    m_data.free_func(m_data.bytes);
                }
                m_data = other.m_data;
                other.m_data = {};
            }
            return *this;
        }

        [[nodiscard]] sandbox_payload* get() { return &m_data; }
        [[nodiscard]] const sandbox_payload* get() const { return &m_data; }

        [[nodiscard]] const uint8_t* data() const { return static_cast<const uint8_t*>(m_data.bytes); }
        [[nodiscard]] size_t size() const { return m_data.size; }

        [[nodiscard]] std::string as_string() const {
            if (!m_data.bytes || m_data.size == 0) return {};
            return std::string(reinterpret_cast<const char*>(m_data.bytes), m_data.size);
        }

    private:
        sandbox_payload m_data{};
    };

} // namespace sandbox::sdk
