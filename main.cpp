#include <iostream>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "HttpSession.hpp"
#include "HttpServ.hpp"

int main(int argc, char* argv[]) {
    namespace http = boost::beast::http;
    namespace net = boost::asio;
    using tcp = net::ip::tcp;

    net::io_context ioc;
    tcp::endpoint endpoint(tcp::v4(), 8080);

    VeloxServ::HttpServ server(ioc, endpoint);
}
