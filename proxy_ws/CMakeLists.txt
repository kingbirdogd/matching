set(CMAKE_CXX_STANDARD 17)
set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/usr/local/lib64/pkgconfig:$ENV{PKG_CONFIG_PATH}")

add_executable(proxy_lws proxy_lws.cpp ./transceiver.cpp ./workqueue.cpp
               proxy_lws_protocol.hpp Uplink.cpp Client.cpp ./core.cpp
               Book.cpp proxy_lws_utils.cpp proxy_lws_protocol.cpp)
target_compile_definitions(proxy_lws PRIVATE VERSION="${VERSION}")
target_include_directories(proxy_lws PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}
        $ENV{HOME}/usr/local/include
        )
target_link_directories(proxy_lws PUBLIC $ENV{HOME}/usr/local/lib64 $ENV{HOME}/usr/local/lib)
target_link_libraries(proxy_lws common tcmalloc_minimal pthread gmp websockets ssl
                      crypto glog folly dl double-conversion)
