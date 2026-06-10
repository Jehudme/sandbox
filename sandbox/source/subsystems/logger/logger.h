#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/subsystems/logger/ilogger.h"
#include "sandbox/utilities/properties.h"
#include "sandbox/subsystems/logger/ilogger.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace sandbox::modules {

    class logger : public ilogger {
    public:
        logger(world& ecs);
        ~logger() override;

        int32_t log(const uint8_t* log_msg_fb, size_t size) override;

        void set_property(const char* key, const char* json_value) override;
        int32_t get_property(const char* key, sandbox_payload* out_payload) const override;

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        bool m_throw_on_error{false};
    };

} // namespace sandbox::modules
