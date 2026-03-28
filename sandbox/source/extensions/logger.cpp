#include "sandbox/extensions/caches.h"
#include <flecs.h>

namespace sandbox::extensions {

    void caches::initialize(const properties& properties) {


    }

    void caches::finalize() {
        stored_entities.clear();
    }

    flecs::entity caches::get(std::string_view entity_name) const {
        auto iterator = stored_entities.find(std::string(entity_name));
        if (iterator != stored_entities.end()) {
            return iterator->second;
        }
        return flecs::entity::null();
    }

    flecs::entity caches::set(flecs::entity entity_handle) {
        stored_entities[entity_handle.get] = entity_handle;
        return entity_handle;
    }

    void caches::remove(flecs::entity entity_handle) {
        stored_entities.erase(std::string(entity_handle.name()));
    }

    void caches::clear() {
        stored_entities.clear();
    }

}