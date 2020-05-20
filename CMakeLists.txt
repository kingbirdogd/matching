cmake_minimum_required (VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 17)
set(VERSION dev)

find_package(PkgConfig REQUIRED)
include_directories(SYSTEM $ENV{HOME}/usr/local/include)

#pkg_check_modules(RABBITMQ REQUIRED IMPORTED_TARGET librabbitmq)

project(engine)
#add_subdirectory(contrib)
add_subdirectory(common)

add_subdirectory(matching)
add_subdirectory(matching_test)
add_subdirectory(matching_tcp_client)
add_subdirectory(matching_tcp_service)

add_subdirectory(matching_zmq_service)

add_subdirectory(net)

add_subdirectory(proxy_ws)

add_subdirectory(test_tcp_matching_client)
add_subdirectory(test_tcp_matching_server)

add_subdirectory(test_zmq_matching_server)
add_subdirectory(xpubxsub)

