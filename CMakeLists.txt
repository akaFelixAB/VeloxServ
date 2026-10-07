cmake_minimum_required(VERSION 4.3.0)
project(VeloxServ VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Code Quality
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY 
        "${CLANG_TIDY_EXE};-header-filter=${CMAKE_SOURCE_DIR}/src/.*"
    )
endif()

# Deps
find_package(Boost REQUIRED COMPONENTS asio beast url)
find_package(tomlplusplus REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON)
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON) 

add_executable(VeloxServ src/main.cpp
    src/HttpSession.cpp
    src/HttpServer.cpp
    src/ConfigManager.cpp)
target_link_libraries(VeloxServ PRIVATE 
    Boost::asio 
    Boost::beast
    Boost::url
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
    spdlog::spdlog)

include(CTest)
enable_testing()

add_executable(smoke_test tests/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE 
    Boost::asio 
    Boost::beast
    Boost::url
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
    spdlog::spdlog)

add_test(NAME SmokeTest COMMAND smoke_test)
set_property(TEST SmokeTest PROPERTY LABELS smoke)


# if(WIN32)
#     # Windows 
#     install(TARGETS VeloxServ DESTINATION bin)
#     install(FILES default.toml DESTINATION bin)
# else()
#     # POSIX
#     install(TARGETS VeloxServ DESTINATION bin)
#     install(FILES default.toml DESTINATION /etc/VeloxServ)
# endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
