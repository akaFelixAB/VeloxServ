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

#include <filesystem>

#include "HttpSession.hpp"

#include <spdlog/spdlog.h>

VeloxServ::http::message_generator VeloxServ::HttpServer::handle_static_request(
    const http::request<http::string_body>& req, 
    const std::string& root_dir,    
    const std::string& index_file
) {
    // Parse the request target (e.g., "/path/to/file?query=1")
    auto result = boost::urls::parse_origin_form(req.target());
    if (result.has_error()) {
        spdlog::warn("Invalid URL format: {}", req.target());
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "Invalid Request Target";
        res.prepare_payload();
        return res;
    }

    boost::urls::url_view uv = result.value();

    spdlog::debug("Parsed URL: {}", uv.buffer());
    
    // Get the path and query components (e.g., "/path/to/file" and "query=1")
    // std::string path = uv.path();
    // std::string query = uv.query();

    // CHeck for ".." in the path segments to prevent directory traversal
    bool contains_dot_dot = false;
    for (auto seg : uv.segments()) {
        if (seg == "..") {
            contains_dot_dot = true;
            break;
        }
    }

    if (contains_dot_dot) {
        spdlog::warn("Illegal request-target detected: {}", req.target());
        return make_error_response(
            http::status::bad_request, 
            req.version(), 
            "Illegal request-target", 
            req.keep_alive()
        );
    }

    // Extract the pure Path component (ignore Query parameters)
    std::string req_path = uv.path();

    // Handle the case where the request path is empty or ends with a slash, indicating a directory
    if (req_path.empty() || req_path.back() == '/') {
        req_path += index_file;
    }

    // Construct the full file path by combining the root directory and the request path
    // Convert into relative path to prevent filesystem traversal
    std::filesystem::path rel_path = req_path.starts_with('/') ? req_path.substr(1) : req_path;
    std::filesystem::path base_path = std::filesystem::weakly_canonical(root_dir);
    std::filesystem::path target_path = std::filesystem::weakly_canonical(base_path / rel_path);

    // Check if the target path is still within the base path to prevent directory traversal
    auto [root_end, dummy] = std::mismatch(base_path.begin(), base_path.end(), target_path.begin());
    if (root_end != base_path.end()) {
        spdlog::warn("Path traversal attempt blocked: {}", target_path.string());
        http::response<http::string_body> res{http::status::forbidden, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "403 Forbidden";
        res.prepare_payload();
        return res;
    }

    std::string full_path = target_path.string();

    // Read the file and prepare the response
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(full_path.c_str(), beast::file_mode::scan, ec);

    if (ec == beast::errc::no_such_file_or_directory) {
        spdlog::warn("File not found: {}", full_path);
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "404 Not Found";
        res.prepare_payload();
        return res;
    }

    if (ec) {
        spdlog::error("Server Error: {}", ec.message());
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "Server Error: " + ec.message();
        res.prepare_payload();
        return res;
    }

    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())
    };

    res.set(http::field::content_type, mime_type(full_path));
    res.keep_alive(req.keep_alive());
    res.prepare_payload();

    return res;
}

void VeloxServ::HttpServer::load_routes_from_config(const ServerConfig& config) {
    spdlog::info("Loading routes from configuration");
    for (const auto& r : config.routes) {
        spdlog::info("Loading route: {}", r.path);
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
    spdlog::info("New connection accepted");
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
