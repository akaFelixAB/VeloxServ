#pragma once

#include "ConfigManager.hpp"

#include <memory>

namespace spdlog {
class logger;  // NOLINT
}

namespace VeloxServ {

class Logger final {
public:
    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void initialize_bootstrap();
    void initialize(const ServerConfig& config);
    void shutdown();

    [[nodiscard]] bool is_initialized() const noexcept {
        return initialized_;
    }

private:
    std::shared_ptr<spdlog::logger> logger_;
    bool initialized_ = false;
};

}  // namespace VeloxServ