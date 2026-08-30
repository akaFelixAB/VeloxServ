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

namespace VeloxServ {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Route handler type: takes a request and returns a response
using Handler = std::function<http::response<http::string_body>(const http::request<http::string_body>&)>;

class HttpServ : public std::enable_shared_from_this<HttpServ> {
    tcp::acceptor _acceptor;
    std::map<std::string, Handler> _routes;

public:
    HttpServ(net::io_context& ioc, tcp::endpoint endpoint)
        : _acceptor(ioc, endpoint) {}

    void route(const std::string& path, Handler handler) {
        _routes[path] = std::move(handler);
    }

    void run() { accept_request(); }

private:
    // Accept a new connection
    void on_accept(boost::system::error_code ec, tcp::socket socket);
    void accept_request();
}; // class HttpServ

} // namespace VeloxServ