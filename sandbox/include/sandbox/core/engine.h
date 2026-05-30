#pragma once
#include "ecs.h"
#include "properties.h"


namespace sandbox {

    class engine {
    public:
        engine(const properties& manifest);
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const properties& manifest);
        void finalize();

    public:
        world ecs;
    };

}
