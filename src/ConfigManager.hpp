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

#include <toml++/toml.hpp>

namespace VeloxServ {

// Overload operator"" _MB
constexpr size_t operator""_MB(unsigned long long mb) {
    return mb * 1024ULL * 1024ULL; // A megabyte is 1024 * 1024 bytes
}

// Route configuration
struct RouteConfig {
    std::string path;
    std::string type;       // "static", "proxy", etc.
    std::string root;       // Static file root directory
    std::string index;      // Default index file
    std::string upstream;   // Reverse proxy target address
};

struct LoggingConfig {
    bool console_output = true;                 // Enable console logging by default
    bool file_output = true;                    // Enable file logging by default
    std::string log_file = "logs/serv.log";
    size_t max_file_size = 10_MB;               // Maximum file size is 10 MB
    int max_files = 3;
};

// Server configuration
struct ServerConfig {
    std::string name = "VeloxServ"; // Server name
    std::string host = "127.0.0.1";
    unsigned short port = 8080;
    int timeout_seconds = 30;
    int max_connections = 10000;
    std::vector<RouteConfig> routes;
    LoggingConfig logging; // Logging configuration
};

class ConfigManager {
private:
    ServerConfig _config;

public:
    ConfigManager() = default;
    ~ConfigManager() = default;

    void parse_file(const std::string& file_path);

    const ServerConfig& get_config() const { return _config; }
}; // class ConfigManager

} // namespace VeloxServ
