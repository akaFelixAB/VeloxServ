#pragma once

#include <boost/beast/http.hpp>

#include <string_view>

namespace VeloxServ {

namespace http = boost::beast::http;
using response = http::response<http::string_body>;

class ResponseFactory final {
public:
    [[nodiscard]] static response error(
        http::status status,
        unsigned version,
        bool keep_alive = false,
        std::string_view message = "Internal Server Error"
    );

    [[nodiscard]] static response bad_request(
        unsigned version,
        bool keep_alive = false,
        std::string_view message = "400 Bad Request"
    );

    [[nodiscard]] static response forbidden(
        unsigned version,
        bool keep_alive = false,
        std::string_view message = "403 Forbidden"
    );

    [[nodiscard]] static response not_found(
        unsigned version,
        bool keep_alive = false,
        std::string_view message = "404 Not Found"
    );

    [[nodiscard]] static response internal_server_error(
        unsigned version,
        bool keep_alive = false,
        std::string_view message = "Internal Server Error"
    );
};

}  // namespace VeloxServ