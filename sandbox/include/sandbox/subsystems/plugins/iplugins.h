#pragma once
#include <expected>
#include <string>

#include <string_view>

namespace sandbox {

    class iplugins {
    public:
        virtual ~iplugins() = default;
        [[nodiscard]] virtual std::expected<void, std::string> load(std::string_view virtual_path, std::string_view entry_point = "SandboxLibraryMain") = 0;
    };

    struct plugins_service {
        iplugins* api{nullptr};
    };

} // namespace sandbox
