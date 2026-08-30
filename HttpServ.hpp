#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <map>

namespace VeloxServ {

namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Route handler type: takes a request and returns a response
using Handler = std::function<http::response<http::string_body>(const http::request<http::string_body>&)>;

class HttpServ {
    tcp::acceptor _acceptor;
    std::map<std::string, Handler> _routes;

public:
    HttpServ(net::io_context& ioc, tcp::endpoint endpoint)
        : _acceptor(ioc, endpoint) {}

    void route(const std::string& path, Handler handler) {
        _routes[path] = std::move(handler);
    }

    void run() { do_accept(); }

private:
    // Accept a new connection
    void do_accept();
}; // class HttpServ

} // namespace VeloxServ