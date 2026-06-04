#pragma once

#include <string_view>

namespace sandbox {

    class iplugins {
    public:
        virtual ~iplugins() = default;
        virtual void load(std::string_view virtual_path, std::string_view entry_point = "SandboxLibraryMain") = 0;
    };

    struct plugins_service {
        iplugins* api{nullptr};
    };

} // namespace sandbox
