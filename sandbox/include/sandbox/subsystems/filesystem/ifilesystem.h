#pragma once

#include <filesystem>
#include <vector>
#include <string_view>
#include <string>
#include <expected>
#include <expected>
#include "sandbox/event_bus/filesystem_events.h"

namespace sandbox {

    class ifilesystem {
    public:
        virtual ~ifilesystem() = default;

        [[nodiscard]] virtual std::expected<void, std::string> mount(std::string_view physical_path, std::string_view virtual_prefix, bool read_only = true) = 0;
        [[nodiscard]] virtual std::expected<void, std::string> unmount(std::string_view virtual_prefix) = 0;
        [[nodiscard]] virtual std::expected<std::vector<std::byte>, std::string> read(std::string_view virtual_path) const = 0;
        [[nodiscard]] virtual std::expected<void, std::string> write(std::string_view virtual_path, std::vector<std::byte> data, bool append = false) = 0;
        [[nodiscard]] virtual std::expected<std::vector<std::filesystem::path>, std::string> list(std::string_view virtual_path, bool recursive = false) const = 0;
        [[nodiscard]] virtual std::expected<void, std::string> remove(std::string_view virtual_path) = 0;
        [[nodiscard]] virtual std::expected<void, std::string> mkdir(std::string_view virtual_path) = 0;
        [[nodiscard]] virtual std::expected<void, std::string> rename(std::string_view old_virtual_path, std::string_view new_virtual_path) = 0;
        [[nodiscard]] virtual std::expected<void, std::string> copy(std::string_view source_virtual_path, std::string_view destination_virtual_path) = 0;
        [[nodiscard]] virtual std::expected<void, std::string> move(std::string_view source_virtual_path, std::string_view destination_virtual_path) = 0;
        [[nodiscard]] virtual std::expected<events::filesystem::file_metadata, std::string> state(std::string_view virtual_path) const = 0;
        [[nodiscard]] virtual std::expected<std::filesystem::path, std::string> absolute(std::string_view virtual_path) const = 0;
    };

    struct filesystem_service {
        ifilesystem* api{nullptr};
    };

} // namespace sandbox
