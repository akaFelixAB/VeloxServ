// Copyright 2026 Felix Huang

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.



#pragma once

#include <chrono>

#include <fmt/chrono.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Custom JSON Formatter
class JsonFormatter : public spdlog::formatter {
public:
    void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override {
        std::string_view lvl{spdlog::level::to_string_view(msg.level).data(), spdlog::level::to_string_view(msg.level).size()};
        std::string_view logger{msg.logger_name.data(), msg.logger_name.size()};
        std::string_view payload{msg.payload.data(), msg.payload.size()};

        // Format the log message as JSON and append it to the destination buffer
        fmt::format_to(
            std::back_inserter(dest),
            "{{\"timestamp\":\"{:%Y-%m-%dT%H:%M:%S.%eZ}\",\"level\":\"{}\",\"logger\":\"{}\",\"message\":\"{}\",\"thread_id\":{}}}\n",
            msg.time,
            lvl,
            logger,
            payload,
            msg.thread_id
        );
    }

    std::unique_ptr<formatter> clone() const override {
        return spdlog::details::make_unique<JsonFormatter>();
    }
};

void init_bootstrap_logging() {
    // Initialize a basic console logger for early logging
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [BOOTSTRAP] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::info);
    
    spdlog::info("Starting VeloxServ pre-flight sequence...");
}

void init_logging(const VeloxServ::ServerConfig& config) {
    // Init thread pool (queue 8192 items, 1 background thread)
    spdlog::init_thread_pool(8192, 1);

    std::vector<spdlog::sink_ptr> sinks;

    // Dynamic file Sink
    if (config.logging.file_output) {
        // Create a rotating file sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            config.logging.log_file, 
            config.logging.max_file_size, 
            config.logging.max_files
        );
        // Set the file sink to use the custom JSON formatter
        file_sink->set_formatter(std::make_unique<JsonFormatter>());
        sinks.push_back(file_sink);
    }

    // Dynamic console Sink
    if (config.logging.console_output) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // Set the console sink to use a human-readable colored format
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
        sinks.push_back(console_sink);
    }

    // Create an asynchronous logger with dynamic sinks
    auto logger = std::make_shared<spdlog::async_logger>(
        config.name, // Use log_file as the logger name
        sinks.begin(), sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::overrun_oldest
    );

    // Set the logger level to info
    logger->set_level(spdlog::level::info); 

    // If encountering an error, flush immediately to ensure the message is logged
    logger->flush_on(spdlog::level::err);

    // Register the logger and set it as the default logger
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

void shutdown_logging() {
    spdlog::shutdown();
}
