#include "ConfigManager.hpp"

#include <filesystem>

#include <iostream>

void VeloxServ::ConfigManager::parse_file(const std::string& file_path) {
    try {
        // Parse the TOML file
        auto tbl = toml::parse_file(file_path);

        // Parse the server configuration
        if (auto server_node = tbl["server"].as_table()) {
            _config.host = server_node->get("host")->value_or("127.0.0.1");
            _config.port = static_cast<unsigned short>(server_node->get("port")->value_or(8080));
            _config.timeout_seconds = static_cast<int>(server_node->get("timeout_seconds")->value_or(30));
            _config.max_connections = static_cast<int>(server_node->get("max_connections")->value_or(10000));
        }

        // Parse the routes configuration
        if (auto routes_node = tbl["routes"].as_array()) {
            _config.routes.clear();
            for (const auto& elem : *routes_node) {
                if (auto route_tbl = elem.as_table()) {
                    RouteConfig route;
                    route.path = (*route_tbl)["path"].value_or("");
                    route.type = (*route_tbl)["type"].value_or("static");
                    route.root = (*route_tbl)["root"].value_or("./public");
                    route.index = (*route_tbl)["index"].value_or("index.html");
                    route.upstream = (*route_tbl)["upstream"].value_or("");

                    if (!route.path.empty()) {
                        _config.routes.push_back(route);
                    }
                }
            }
        }
    } catch (const toml::parse_error& err) {
        throw std::runtime_error("TOML Parse Error: " + std::string(err.description()) 
                                 + " at line " + std::to_string(err.source().begin.line));
    } catch (const std::exception& e) {
        throw std::runtime_error("Config Load Failed: " + std::string(e.what()));
    }
}