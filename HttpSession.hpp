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

namespace VeloxServ {

namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Route handler type: takes a request and returns a response
using Handler = std::function<http::response<http::string_body>(const http::request<http::string_body>&)>;

// Manages the lifetime of an HTTP session for a single connection
class HttpSession : public std::enable_shared_from_this<HttpSession> {
    tcp::socket _socket;                            // Socket for the session
    http::request<http::string_body> _request;      // Request received from the client
    http::response<http::string_body> _response;    // Response to be sent
    boost::beast::flat_buffer _buffer;              // Buffer for reading
    std::map<std::string, Handler> _routes;         // Map of routes to handlers

public:
    HttpSession(tcp::socket socket, std::map<std::string, Handler> routes)
        : _socket(std::move(socket)), _routes(std::move(routes)) {}

    inline void start() { read_request(); }

private:
    // State machine for handling the HTTP session

    // Read the request from the client
    void read_request();

    // Process the request and generate a response
    void process_request();

    // Write the response back to the client
    void write_response();
}; // class HttpSession

} // namespace VeloxServ