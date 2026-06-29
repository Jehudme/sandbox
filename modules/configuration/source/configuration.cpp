#include "configuration.h"



namespace sandbox::modules {

    configuration::configuration(ecs_world_t* ecs) {
        flecs::world world(ecs);
        flecs::entity properties_handle_entity = world.entity("sandbox::configuration::handle");

        auto properties_handle = properties_handle_entity.get<>();



    }

    configuration::~configuration() {
        // Destructor logic if needed
    }

}