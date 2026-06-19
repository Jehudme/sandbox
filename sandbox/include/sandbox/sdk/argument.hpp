#pragma once
#include "sandbox/abi/argument.h"
#include "properties.hpp"
#include <string>
#include <vector>

namespace sandbox {
    class Argument {
    public:
        static bool has(ecs_world_t* ecs, const std::string& path);
        
        static bool get_int64(ecs_world_t* ecs, const std::string& path, int64_t& out_val);
        static bool get_double(ecs_world_t* ecs, const std::string& path, double& out_val);
        static bool get_bool(ecs_world_t* ecs, const std::string& path, bool& out_val);
        static bool get_string(ecs_world_t* ecs, const std::string& path, std::string& out_val);
        
        static std::vector<std::string> keys(ecs_world_t* ecs, const std::string& path);
        static Properties get_subtree(ecs_world_t* ecs, const std::string& path);
    };
}

#include "detail/argument.inl"
