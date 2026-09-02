cmake_minimum_required(VERSION 3.10.0)
project(VeloxServ VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost
find_package(Boost REQUIRED COMPONENTS asio beast)
find_package(tomlplusplus REQUIRED)
find_package(spdlog REQUIRED)

add_executable(VeloxServ main.cpp
    HttpSession.cpp
    HttpServer.cpp
    ConfigManager.cpp)

target_link_libraries(VeloxServ PRIVATE 
    Boost::asio 
    Boost::beast
    tomlplusplus::tomlplusplus
    spdlog::spdlog)

include(CTest)
enable_testing()


# if(WIN32)
#     # Windows 
#     install(TARGETS VeloxServ DESTINATION bin)
#     install(FILES default.toml DESTINATION bin) # 或者放在 config/
# else()
#     # POSIX
#     install(TARGETS VeloxServ DESTINATION bin)
#     install(FILES default.toml DESTINATION /etc/VeloxServ)
# endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
