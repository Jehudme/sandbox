#pragma once

#include "sandbox/core/extension.h"
#include <string_view>
#include <functional>
#include <memory>

#include "flecs/addons/cpp/entity.hpp"
namespace sandbox::extensions
{
    class events : public sandbox::extension
    {
    public:
        template<typename event_t>
        using event_callback_t = std::function<void(const event_t&)>;

        events() = default;
        ~events() override = default;

        void initialize(const properties& configuration) override;
        void finalize() override;

        template<typename event_t>
        void publish(event_t&& event);

        template<typename event_t>
        void create_observer(std::string_view name, flecs::entity target, event_callback_t<event_t>&& callback);
        void destroy_observer(std::string_view name, flecs::entity target);

        void enable_observer(std::string_view name, flecs::entity target);
        void disable_observer(std::string_view name, flecs::entity target);

        bool is_observer_enabled(std::string_view name, flecs::entity target) const;
        bool is_observer_exist(std::string_view name, flecs::entity target) const;

    private:
        struct transient_tag{};
    };
}

#include "events.inl"