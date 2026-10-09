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