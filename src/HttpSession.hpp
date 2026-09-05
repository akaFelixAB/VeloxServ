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

#include <memory>
#include <map>
#include <functional>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/url.hpp>

namespace VeloxServ {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace urls = boost::urls;
using tcp = net::ip::tcp;

// Route handler type: takes a request and returns a response
using Handler = std::function<http::message_generator(const http::request<http::string_body>&)>;
// Type alias for route table
using RouteTable = std::map<std::string, Handler>;

inline http::response<http::string_body> make_error_response(
    http::status status, 
    unsigned version, 
    std::string_view message, 
    bool keep_alive = false
) {
    http::response<http::string_body> res{status, version};
    res.set(http::field::content_type, "text/plain");
    res.keep_alive(keep_alive);
    res.body() = message;
    res.prepare_payload();
    return res;
}

// Manages the lifetime of an HTTP session for a single connection
class HttpSession : public std::enable_shared_from_this<HttpSession> {
    beast::tcp_stream _socket;                          // Socket for the session
    http::request<http::string_body> _request;          // Request received from the client
    beast::flat_buffer _buffer;                         // Buffer for reading
    std::shared_ptr<const RouteTable> _routes;             // Map of routes to handlers

public:
    HttpSession(tcp::socket socket, std::shared_ptr<const RouteTable> routes)
    : _socket(std::move(socket)), _routes(std::move(routes)) {}

    inline void start() { read_request(); }

    ~HttpSession() = default;

private:
    // State machine for handling the HTTP session

    // Read the request from the client
    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);
    void read_request();

    // Process the request and generate a response and send it back to the client
    void process_request();
}; // class HttpSession

} // namespace VeloxServ