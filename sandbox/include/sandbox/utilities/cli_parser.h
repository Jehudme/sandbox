#pragma once

#include <string>
#include <string_view>
#include <stdexcept>
#include <cstdint>
#include "sandbox/core/engine.h"

namespace sandbox {
namespace utilities {

inline sandbox::activation_request parse_activation_argument(std::string_view arg) {
    sandbox::activation_request req;
    auto colon_pos = arg.find(':');
    if (colon_pos == std::string_view::npos) {
        req.module_name = std::string(arg);
        return req;
    }
    
    req.module_name = std::string(arg.substr(0, colon_pos));
    std::string_view version_str = arg.substr(colon_pos + 1);
    
    auto parse_strict_int = [](std::string_view s) -> uint8_t {
        if (s.empty()) throw std::invalid_argument("empty component");
        size_t parsed_chars = 0;
        int val = std::stoi(std::string(s), &parsed_chars);
        if (parsed_chars != s.length()) throw std::invalid_argument("extra characters");
        return static_cast<uint8_t>(val);
    };

    auto dot_pos1 = version_str.find('.');
    if (dot_pos1 == std::string_view::npos) {
        throw std::invalid_argument("Malformed activation version string: expected major.minor or major.minor.patch");
    }
    
    auto dot_pos2 = version_str.find('.', dot_pos1 + 1);
    
    try {
        req.major = parse_strict_int(version_str.substr(0, dot_pos1));
        if (dot_pos2 == std::string_view::npos) {
            req.minor = parse_strict_int(version_str.substr(dot_pos1 + 1));
        } else {
            req.minor = parse_strict_int(version_str.substr(dot_pos1 + 1, dot_pos2 - dot_pos1 - 1));
            auto dot_pos3 = version_str.find('.', dot_pos2 + 1);
            if (dot_pos3 != std::string_view::npos) {
                throw std::invalid_argument("too many dots");
            }
            req.patch = parse_strict_int(version_str.substr(dot_pos2 + 1));
        }
    } catch (const std::exception&) {
        throw std::invalid_argument("Malformed activation version string: unable to parse integer parts");
    }
    
    return req;
}

} // namespace utilities
} // namespace sandbox
