#include "ResponseFactory.hpp"

namespace VeloxServ {

response ResponseFactory::error(
    http::status status,
    unsigned version,
    bool keep_alive,
    std::string_view message
) {
    response res{status, version};
    res.set(http::field::content_type, "text/plain; charset=utf-8");
    res.keep_alive(keep_alive);
    res.body() = message;
    res.prepare_payload();
    return res;
}

response ResponseFactory::bad_request(
    unsigned version,
    bool keep_alive,
    std::string_view message
) {
    return error(http::status::bad_request, version, keep_alive, message);
}

response ResponseFactory::forbidden(
    unsigned version,
    bool keep_alive,
    std::string_view message
) {
    return error(http::status::forbidden, version, keep_alive, message);
}

response ResponseFactory::not_found(
    unsigned version,
    bool keep_alive,
    std::string_view message
) {
    return error(http::status::not_found, version, keep_alive, message);
}

response ResponseFactory::internal_server_error(
    unsigned version,
    bool keep_alive,
    std::string_view message
) {
    return error(http::status::internal_server_error, version, keep_alive, message);
}

}  // namespace VeloxServ