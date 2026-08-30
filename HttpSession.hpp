#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <map>
#include <functional>

namespace VeloxServ {

namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Route handler type: takes a request and returns a response
using Handler = std::function<http::response<http::string_body>(const http::request<http::string_body>&)>;


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
    void read_request();

    void process_request();

    void write_response();
}; // class HttpSession

} // namespace VeloxServ