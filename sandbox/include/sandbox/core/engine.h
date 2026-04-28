#pragma once
#include <string_view>

#include "sandbox/core/ecs.h"
#include "sandbox/utils/properties.h"

namespace sandbox {

class engine {
public:
    engine() = default;
    ~engine() = default;

    void initialize(const properties& manifest);
    void finalize();

    void create_plugin(std::string_view alias, std::string_view type_name);
    void delete_plugin(std::string_view alias);

    template<typename plugin_type>
    plugin_type* find_plugin(std::string_view alias);

private:
    world m_world;
};

}
