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

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <memory>
#include <map>

#include "ConfigManager.hpp"

namespace VeloxServ {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Route handler type: takes a request and returns a response
using Handler = std::function<http::message_generator(const http::request<http::string_body>&)>;

class HttpServer {
    private:
    // The Pimpl idiom: hide implementation details in a separate class
    class Impl : public std::enable_shared_from_this<Impl> {
        tcp::acceptor _acceptor;
        ServerConfig _config;
        std::map<std::string, Handler> _routes;

    public:
        Impl(net::io_context& ioc, const ServerConfig& config) 
        : _acceptor(ioc, tcp::endpoint(net::ip::make_address(config.host), config.port)),
          _config(std::move(config)) {}

        void route(const std::string& path, Handler handler) {
            _routes[path] = std::move(handler);
        }

        void run() { accept_request(); }

    private:
        void on_accept(boost::system::error_code ec, tcp::socket socket);
        void accept_request();
    }; // class Impl

    // Manage shared_ptr of Impl to ensure proper lifetime management
    std::shared_ptr<Impl> _pimpl;

    // Get the MIME type based on the file extension
    inline beast::string_view mime_type(beast::string_view path) {
        using beast::iequals;
        auto const ext = [&path] {
            auto const pos = path.rfind(".");
            if(pos == beast::string_view::npos) return beast::string_view{};
            return path.substr(pos);
        }();
        if(iequals(ext, ".htm"))  return "text/html";
        if(iequals(ext, ".html")) return "text/html";
        if(iequals(ext, ".php"))  return "text/html";
        if(iequals(ext, ".css"))  return "text/css";
        if(iequals(ext, ".txt"))  return "text/plain";
        if(iequals(ext, ".js"))   return "application/javascript";
        if(iequals(ext, ".json")) return "application/json";
        if(iequals(ext, ".png"))  return "image/png";
        if(iequals(ext, ".jpe"))  return "image/jpeg";
        if(iequals(ext, ".jpeg")) return "image/jpeg";
        if(iequals(ext, ".jpg"))  return "image/jpeg";
        if(iequals(ext, ".gif"))  return "image/gif";
        if(iequals(ext, ".bmp"))  return "image/bmp";
        if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
        if(iequals(ext, ".svg"))  return "image/svg+xml";
        if(iequals(ext, ".svgz")) return "image/svg+xml";
        return "application/octet-stream"; // Default MIME type for unknown extensions
    }

    // The core static file handling function
    http::message_generator handle_static_request(
        const http::request<http::string_body>& req, 
        const std::string& root_dir, 
        const std::string& index_file
    );

public:
    // Automatically create a HttpServer on the heap and return a shared_ptr to it
    HttpServer(net::io_context& ioc, const ServerConfig& config)
    : _pimpl(std::make_shared<Impl>(ioc, config)) {
        load_routes_from_config(config);
    }

    void load_routes_from_config(const ServerConfig& config);

    // Forward the route and run calls to the Impl instance
    void route(const std::string& path, Handler handler) {
        _pimpl->route(path, std::move(handler));
    }

    void run() {
        _pimpl->run();
    }
}; // class HttpServ

} // namespace VeloxServ