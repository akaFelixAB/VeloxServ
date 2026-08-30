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

    _response.version(_request.version());
    _response.keep_alive(_request.keep_alive());

    std::string path = std::string(_request.target());
    auto it = _routes.find(path); // Find the handler for the requested path
    if (it != _routes.end()) {
        try {
            _response = it->second(_request);
        } catch (const std::exception& e) {
            _response.result(http::status::internal_server_error);
            _response.set(http::field::content_type, "text/plain");
            _response.body() = e.what();
        }
    } else {
        _response.result(http::status::not_found);
        _response.set(http::field::content_type, "text/plain");
        _response.body() = "404 Not Found";
    }

    _response.prepare_payload();
    write_response();
}

void VeloxServ::HttpSession::on_write(boost::system::error_code ec, std::size_t bytes_transferred) {
    if (!ec) {
        if (_response.keep_alive()) {
            read_request();
        }
    }
}

void VeloxServ::HttpSession::write_response() {
    auto self = shared_from_this();
    http::async_write(
        _socket, _response,
        beast::bind_front_handler(
            &HttpSession::on_write,
            self
        )
    );
}

void VeloxServ::HttpSession::do_close() {
    _socket.close();
}

VeloxServ::HttpSession::~HttpSession() {
    do_close();
}