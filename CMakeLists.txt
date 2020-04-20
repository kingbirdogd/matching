cmake_minimum_required (VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
#pkg_check_modules(RABBITMQ REQUIRED IMPORTED_TARGET librabbitmq)

#include_directories(matching/inc)

project(engine)
add_subdirectory(contrib)
add_subdirectory(common)

add_subdirectory(matching)
add_subdirectory(matching_test)

add_subdirectory(proxy_ws)

