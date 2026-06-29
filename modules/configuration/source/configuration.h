#pragma once
#include <any>
#include <flecs.h>
#include <optional>
#include <string>
#include <unordered_map>

#include "sandbox/abi/bootstrapper.h"
#include "sandbox/sdk/properties.hpp"



namespace sandbox::modules {
    class configuration {
    public:
        configuration(ecs_world_t* ecs);
        ~configuration();

        template <typename Type>
        std::optional<Type> get(const std::string& key) const;

    private:
        properties m_properties;
    };
    

    template<typename Type>
    std::optional<Type> configuration::get(const std::string &key) const {
        return m_properties.get<Type>(key);
    }
}
