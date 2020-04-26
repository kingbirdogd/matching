add_library(matching_tcp_client matching_tcp_client.cpp )
target_include_directories(matching_tcp_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../inc)
target_include_directories(matching_tcp_client PUBLIC ${CMAKE_SOURCE_DIR}/matching/order/inc)
target_include_directories(matching_tcp_client PUBLIC ${CMAKE_SOURCE_DIR}/net/tcp_client/inc)

