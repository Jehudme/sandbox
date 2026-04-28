#pragma once
#include <string_view>

#include "sandbox/core/ecs.h"
#include "sandbox/utils/properties.h"

namespace sandbox {

class engine {
public:
    engine() = default;
    ~engine() = default;

    void initialize(const properties& properties);
    void finalize();

    void create_plugin(std::string_view name, std::string_view identifier);
    void delete_plugin(std::string_view name);

    template<typename plugin_type>
    plugin_type* find_plugin(std::string_view name);

    const properties& get_properties() const;

private:
    world m_world;
};

}
