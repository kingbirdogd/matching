cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

set(VERSION dev)
set(SUBMODULE_VERSION dev)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_BUILD_TYPE Debug)
execute_process(COMMAND git describe --always --dirty OUTPUT_VARIABLE VERSION)
string(STRIP "${VERSION}" VERSION)
message("VERSION = ${VERSION}")

execute_process(COMMAND git submodule status --recursive OUTPUT_VARIABLE SUBMODULE_VERSION)
string(STRIP "${SUBMODULE_VERSION}" SUBMODULE_VERSION)
message("SUBMODULE_VERSION = ${SUBMODULE_VERSION}")

find_package(PkgConfig REQUIRED)
include_directories(SYSTEM $ENV{HOME}/usr/local/include)
link_directories(SYSTEM $ENV{HOME}/usr/local/lib $ENV{HOME}/usr/local/lib64)

#pkg_check_modules(RABBITMQ REQUIRED IMPORTED_TARGET librabbitmq)

project(engine)
#add_subdirectory(contrib)
add_subdirectory(coinflex_v2_common)
add_subdirectory(common)

add_subdirectory(matching)
add_subdirectory(matching_test)
add_subdirectory(matching_tcp_client)
add_subdirectory(matching_tcp_service)
add_subdirectory(md_tcp_service)

add_subdirectory(matching_zmq_service)

add_subdirectory(net)

#add_subdirectory(proxy_ws)
#add_subdirectory(proxy_md_ws)
#add_subdirectory(zmq_proxy)

add_subdirectory(test_tcp_matching_client)
add_subdirectory(auction_tcp_matching_client)
add_subdirectory(test_tcp_matching_server)
add_subdirectory(test_md_tcp_server)

add_subdirectory(test_zmq_matching_server)
add_subdirectory(xpubxsub)

add_subdirectory(md)
add_subdirectory(pulsar)
add_subdirectory(pulsar_proxy)
add_subdirectory(proxy_md_pulsar)
add_subdirectory(config_parser)