#pragma once
#include "ecs.h"
#include "sandbox/utils/properties.h"


namespace sandbox {

    class engine {
    public:
        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const properties& manifest);
        void finalize();

    private:
        void load_plugin(std::string_view type_name);
        void unload_plugin(std::string_view type_name);

    public:
        world ecs;
    };

}
