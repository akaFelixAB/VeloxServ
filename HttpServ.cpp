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
