add_executable(proxy_ws proxy_lws.cpp transceiver.cpp workqueue.cpp
               proxy_lws_protocol.hpp Uplink.cpp Client.cpp core.cpp
               Book.cpp proxy_lws_utils.cpp)
target_compile_definitions(proxy_ws PRIVATE VERSION="${VERSION}")
target_include_directories(proxy_ws PUBLIC ${CMAKE_SOURCE_DIR}/matching/inc)
target_include_directories(proxy_ws PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR})
target_link_libraries(proxy_ws common tcmalloc_minimal pthread gmp websockets ssl crypto)
