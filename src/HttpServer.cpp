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



#include "HttpServer.hpp"

#include "HttpSession.hpp"

VeloxServ::http::message_generator VeloxServ::HttpServer::handle_static_request(
    const http::request<http::string_body>& req, 
    const std::string& root_dir,    
    const std::string& index_file
) {
    std::string req_path = std::string(req.target());

    // Safe path check, prevent path traversal attacks (e.g., GET /../../etc/passwd)
    if (req_path.find("..") != std::string::npos) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "Illegal request-target";
        res.prepare_payload();
        return res;
    }

    // Normalize the request path to ensure it starts with a '/'
    std::string full_path = root_dir + req_path;

    // If the request path ends with '/', append the index file
    if (full_path.back() == '/') {
        full_path += index_file;
    }

    // Try to open the file using Boost.Beast's file_body
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(full_path.c_str(), beast::file_mode::scan, ec);

    // Handle errors when opening the file
    if (ec == beast::errc::no_such_file_or_directory) {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "File not found";
        res.prepare_payload();
        return res;
    }
    if (ec) {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "Server Error: " + ec.message();
        res.prepare_payload();
        return res;
    }

    // Respond with the file content using http::file_body
    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())
    };

    res.set(http::field::content_type, mime_type(full_path));
    res.keep_alive(req.keep_alive());
    res.prepare_payload();

    // Return the response as a message_generator
    return res;
}

void VeloxServ::HttpServer::load_routes_from_config(const ServerConfig& config) {
    for (const auto& r : config.routes) {
        if (r.type == "static") {
            // Register a static file handler for the route
            _pimpl->route(r.path, 
                [this, r](const http::request<http::string_body>& req) {
                return handle_static_request(req, r.root, r.index);
                }
            );
        } else if (r.type == "proxy") {
            // Register a proxy handler for the route
        }
    }
}

void VeloxServ::HttpServer::Impl::on_accept(boost::system::error_code ec, tcp::socket socket) {
    if (!ec) {
        std::make_shared<VeloxServ::HttpSession>(std::move(socket), _routes)->start();
    }
    accept_request();
}

void VeloxServ::HttpServer::Impl::accept_request() {
    _acceptor.async_accept(
        beast::bind_front_handler(
            &Impl::on_accept,
            shared_from_this() // safe call to shared_from_this()
        )
    );
}
