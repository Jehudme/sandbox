namespace sandbox
{
    template<typename event_type>
    void dispatcher::publish(std::unique_ptr<event_type> event)
    {
        auto it = m_handlers.find(typeid(event_type));
        if (it == m_handlers.end()) return;

        for (const auto& handler : it->second)
        {
            const auto& fn =
                std::any_cast<const std::function<void(const event_type&)>&>(handler);
            fn(*event);
        }
    }

    template<typename event_type>
    void dispatcher::subscribe(std::function<void(const event_type&)> callback)
    {
        m_handlers[typeid(event_type)].emplace_back(std::move(callback));
    }
}
