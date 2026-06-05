#pragma once

#include <unordered_map>
#include <string>
#include <any>

namespace sandbox {

    template<typename T>
    T get_config(const std::unordered_map<std::string, std::any>& map, const std::string& key, T default_value) {
        if (auto it = map.find(key); it != map.end() && it->second.type() == typeid(T)) {
            return std::any_cast<T>(it->second);
        }
        return default_value;
    }

} // namespace sandbox
