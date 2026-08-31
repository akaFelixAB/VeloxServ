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



#include <iostream>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "HttpSession.hpp"
#include "HttpServer.hpp"
#include "ConfigManager.hpp"

int main(int argc, char* argv[]) {
    namespace http = boost::beast::http;
    namespace net = boost::asio;
    using tcp = net::ip::tcp;

    std::string config_path = "default.toml";

    try {
        VeloxServ::ConfigManager config_mgr;
        config_mgr.parse_file(config_path);
        const auto& cfg = config_mgr.get_config();

        std::cout << "[Config] Loaded successfully from " << config_path << "\n";
        std::cout << "[Config] Listening on " << cfg.host << ":" << cfg.port << "\n";

        net::io_context ioc;

        // Create the HTTP server
        VeloxServ::HttpServer server(ioc, cfg);

        server.run();
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
