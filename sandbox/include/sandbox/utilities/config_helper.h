#pragma once

#include <unordered_map>
#include <string>
#include "sandbox/utilities/properties.h"

namespace sandbox {

    template<typename T>
    T get_config(const sandbox::properties& map, const std::string& key, T default_value) {
        if (auto res = map.get<T>({key})) {
            return res.value();
        }
        return default_value;
    }

} // namespace sandbox
