#include "HttpSession.hpp"

void VeloxServ::HttpSession::read_request() {
    auto self = shared_from_this();
    http::async_read(
        _socket, _buffer, _request,
        [self](boost::system::error_code ec, std::size_t
    ) {
        if (!ec) {
            self->process_request();
        }
    });
}

void VeloxServ::HttpSession::process_request() {
    _response.version(_request.version());
    _response.keep_alive(false);

    std::string path = std::string(_request.target());
    auto it = _routes.find(path);
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

void VeloxServ::HttpSession::write_response() {
    auto self = shared_from_this();
    http::async_write(
        _socket, _response,
        [self](boost::system::error_code ec, std::size_t
    ) {
        if (!ec && self->_request.keep_alive()) {
            self->read_request();
        }
    });
}