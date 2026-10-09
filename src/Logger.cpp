#include "Logger.hpp"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <spdlog/async.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

using json = nlohmann::json;

class JsonFormatter final : public spdlog::formatter {
public:
    void format(
        const spdlog::details::log_msg& message,
        spdlog::memory_buf_t& destination
    ) override {
        const auto level = spdlog::level::to_string_view(message.level);
        const std::string level_str {
            level.data(),
            level.size()
        };

        const std::string logger_name {
            message.logger_name.data(),
            message.logger_name.size()
        };

        const std::string payload {
            message.payload.data(),
            message.payload.size()
        };

        const std::string timestamp =
            fmt::format("{:%Y-%m-%dT%H:%M:%S.%e}Z", message.time);

        const json record {
            {"timestamp", timestamp},
            {"level", level_str},
            {"logger", logger_name},
            {"message", payload},
            {"thread_id", message.thread_id}
        };

        fmt::format_to(
            std::back_inserter(destination),
            "{}\n",
            record.dump()
        );
    }

    [[nodiscard]] std::unique_ptr<formatter> clone() const override {
        return spdlog::details::make_unique<JsonFormatter>();
    }
};

}  // namespace

namespace VeloxServ {

Logger::~Logger() {
    shutdown();
}

void Logger::initialize_bootstrap() {
    if (initialized_) {
        return;
    }

    spdlog::set_pattern(
        "[%Y-%m-%d %H:%M:%S] [BOOTSTRAP] [%^%l%$] %v"
    );

    spdlog::set_level(spdlog::level::info);

    spdlog::info("Starting VeloxServ pre-flight sequence...");
}

void Logger::initialize(const ServerConfig& config) {
    if (initialized_) {
        return;
    }

    spdlog::init_thread_pool(8192, 1);

    std::vector<spdlog::sink_ptr> sinks;

    if (config.logging_.file_output_) {
        auto file_sink =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config.logging_.log_file_,
                config.logging_.max_file_size_,
                config.logging_.max_files_
            );

        file_sink->set_formatter(
            std::make_unique<JsonFormatter>()
        );

        sinks.push_back(file_sink);
    }

    if (config.logging_.console_output_) {
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        console_sink->set_pattern(
            "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v"
        );

        sinks.push_back(console_sink);
    }

    if (sinks.empty()) {
        throw std::runtime_error(
            "Logger requires at least one enabled sink"
        );
    }

    auto async_logger = std::make_shared<spdlog::async_logger>(
        config.name_,   // Logger name
        sinks.begin(),  // Begin iterator for sinks
        sinks.end(),    // End iterator for sinks
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::overrun_oldest
    );

    async_logger->set_level(spdlog::level::info);
    async_logger->flush_on(spdlog::level::err);

    spdlog::register_logger(async_logger);
    spdlog::set_default_logger(async_logger);

    logger_ = std::move(async_logger);
    initialized_ = true;

    spdlog::info("Logging initialized");
}

void Logger::shutdown() {
    if (!initialized_) {
        return;
    }

    if (logger_) {
        logger_->flush();
    }

    spdlog::shutdown();

    logger_.reset();
    initialized_ = false;
}

}  // namespace VeloxServ