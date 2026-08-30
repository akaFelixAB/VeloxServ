#include "HttpServ.hpp"
#include "HttpSession.hpp"

void VeloxServ::HttpServ::do_accept() {
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<VeloxServ::HttpSession>(std::move(socket), _routes)->start();
        }
        do_accept();
        }
    );
}
