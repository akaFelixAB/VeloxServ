#pragma once

#include <boost/beast.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace VeloxServ {

namespace http = boost::beast::http;

// Route handler type: takes a request and returns a response
using Handler = std::function<http::message_generator(const http::request<http::string_body>&)>;
// Type alias for route table
using RouteTable = std::map<std::string, Handler>;

}