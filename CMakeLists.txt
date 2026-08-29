cmake_minimum_required(VERSION 3.10.0)
project(VeloxServ VERSION 0.1.0 LANGUAGES C CXX)

# Boost
find_package(Boost REQUIRED COMPONENTS asio beast)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(VeloxServ main.cpp
    HttpServ.cpp)

target_link_libraries(VeloxServ PRIVATE 
    Boost::asio 
    Boost::beast 
    nlohmann_json::nlohmann_json)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
