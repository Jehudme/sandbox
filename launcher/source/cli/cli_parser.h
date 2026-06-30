#pragma once
#include <sandbox/sdk/properties.hpp>
#include <optional>

namespace sandbox::launcher {

    /**
     * @brief Parses the command line arguments and returns a populated properties object.
     * @param argc The argument count.
     * @param argv The argument vector.
     * @return An optional properties object. Empty if parsing failed or exit requested (e.g., --help).
     */
    std::optional<sandbox::properties> parse_cli(int argc, char** argv);

}
