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



#include "HttpSession.hpp"

#include <spdlog/spdlog.h>

void VeloxServ::HttpSession::on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
    if (!ec) {
        process_request();
    }
}

void VeloxServ::HttpSession::read_request() {
    auto self = shared_from_this();
    http::async_read(
        _socket, _buffer, _request,
        beast::bind_front_handler(
            &HttpSession::on_read,
            shared_from_this()
        )
    );
}

void VeloxServ::HttpSession::process_request() {
    _socket.expires_after(std::chrono::seconds(30));

    // Save the keep-alive status and the request path for routing
    bool keep_alive = _request.keep_alive();
    
    // Parse the request target (e.g., "/path/to/file?query=1")
    std::string_view req_target = _request.target();
    auto parsed_url = boost::urls::parse_origin_form(req_target);

    std::string path;
    if (parsed_url.has_value()) {
        spdlog::debug("Parsed URL: {}", parsed_url->buffer());
        path.assign(parsed_url->path().data(), parsed_url->path().size());
    } else {
        path.assign(req_target.data(), req_target.size());
    }

    auto matched_it = _routes->end();
    size_t max_len = 0;

    for (auto it = _routes->begin(); it != _routes->end(); ++it) {
        const std::string& route_prefix = it->first;
        if (path.rfind(route_prefix, 0) == 0) { // Match if the path starts with the route prefix
            if (route_prefix.length() > max_len) {
                max_len = route_prefix.length();
                matched_it = it;
            }
        }
    }

    http::message_generator msg = http::response<http::string_body>{
        http::status::internal_server_error, _request.version()
    }; // Default to 500

    if (matched_it != _routes->end()) {
        try {
            msg = matched_it->second(_request); // Handle the request using the registered handler
        } catch (const std::exception& e) {
            http::response<http::string_body> res{http::status::internal_server_error, _request.version()};
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(keep_alive);
            res.body() = e.what();
            res.prepare_payload();
            msg = std::move(res);
        }
    } else {
        http::response<http::string_body> res{http::status::not_found, _request.version()};
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(keep_alive);
        res.body() = "404 Not Found";
        res.prepare_payload();
        msg = std::move(res);
    }

    // Write the response back to the client
    auto self = shared_from_this();
    beast::async_write(
        _socket,
        std::move(msg),
        [self, keep_alive](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec && keep_alive) {
                self->read_request();
            }
        }
    );
}
