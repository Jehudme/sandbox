#pragma once
#include <string_view>

#include "sandbox/core/ecs.h"
#include "sandbox/utils/properties.h"

namespace sandbox {
    class plugin;

    class engine {
    public:
        engine() = default;
        ~engine();

        void initialize(const properties& manifest);

        void create_plugin(std::string_view alias, std::string_view type_name);
        void delete_plugin(std::string_view alias);
        plugin* get_plugin(std::string_view alias);

        template<typename plugin_type>
        plugin_type* find_plugin(std::string_view alias);

        world ecs;

    private:
        void call_plugin_initialize(plugin* p);
        void call_plugin_finalize(plugin* p);
    };

    template <typename plugin_type>
    plugin_type* engine::find_plugin(std::string_view alias) {
        return dynamic_cast<plugin_type*>(get_plugin(alias));
    }


}
