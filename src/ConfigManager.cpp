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

#include "ConfigManager.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <iostream>

void VeloxServ::ConfigManager::parse_file(const std::string& file_path) {
    try {
        spdlog::info("Loading configuration from {}", file_path);
        // Parse the TOML file
        auto tbl = toml::parse_file(file_path);

        // Parse the server configuration
        if (auto server_node = tbl["server"].as_table()) {
            config_.name_ = server_node->get("name")->value_or("VeloxServ");
            config_.host_ = server_node->get("host")->value_or("127.0.0.1");
            config_.port_ = static_cast<unsigned short>(server_node->get("port")->value_or(8080));
            config_.timeout_seconds_ =
                static_cast<int>(server_node->get("timeout_seconds")->value_or(30));
            config_.max_connections_ =
                static_cast<int>(server_node->get("max_connections")->value_or(10000));
        }

        // Parse the logging configuration
        if (auto logging_node = tbl["logging"].as_table()) {
            config_.logging_.console_output_ =
                static_cast<bool>(logging_node->get("console_output")->value_or(true));
            config_.logging_.file_output_ =
                static_cast<bool>(logging_node->get("file_output")->value_or(true));
            config_.logging_.log_file_ = logging_node->get("log_file")->value_or("logs/serv.log");
            config_.logging_.max_file_size_ =
                static_cast<size_t>(logging_node->get("max_file_size")->value_or(10)) * 1_MB;
            config_.logging_.max_files_ =
                static_cast<int>(logging_node->get("max_files")->value_or(3));
        }

        // Parse the routes configuration
        if (auto routes_node = tbl["routes"].as_array()) {
            config_.routes_.clear();
            for (const auto& elem : *routes_node) {
                if (auto route_tbl = elem.as_table()) {
                    RouteConfig route;
                    route.path_ = (*route_tbl)["path"].value_or("");
                    route.type_ = (*route_tbl)["type"].value_or("static");
                    route.root_ = (*route_tbl)["root"].value_or("./public");
                    route.index_ = (*route_tbl)["index"].value_or("index.html");
                    route.upstream_ = (*route_tbl)["upstream"].value_or("");

                    if (!route.path_.empty()) {
                        config_.routes_.push_back(route);
                    }
                }
            }
        }
    } catch (const toml::parse_error& err) {
        throw std::runtime_error(
            "TOML Parse Error: " + std::string(err.description()) + " at line " +
            std::to_string(err.source().begin.line)
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Config Load Failed: " + std::string(e.what()));
    }
}