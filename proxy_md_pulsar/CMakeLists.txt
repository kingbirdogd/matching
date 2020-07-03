set(CMAKE_CXX_STANDARD 17)
set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/usr/local/lib64/pkgconfig:$ENV{PKG_CONFIG_PATH}")

add_executable(proxy_md_pulsar proxy_lws.cpp ./transceiver.cpp ./workqueue.cpp
               #proxy_lws_protocol.hpp
               Uplink.cpp
               Client.cpp
               ./core.cpp
               OrderBook.cpp
               #proxy_lws_utils.cpp
               #proxy_lws_protocol.cpp
        )
target_compile_definitions(proxy_md_pulsar PRIVATE VERSION="${VERSION}")
target_include_directories(proxy_md_pulsar PUBLIC
        ${CMAKE_SOURCE_DIR}/matching/order/inc/
        ${CMAKE_SOURCE_DIR}/md/inc/
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}
        $ENV{HOME}/usr/local/include
        )
target_link_directories(proxy_md_pulsar PUBLIC $ENV{HOME}/usr/local/lib64 $ENV{HOME}/usr/local/lib)
target_link_libraries(proxy_md_pulsar common pthread gmp websockets ssl
                      crypto glog dl double-conversion flatbuffers pulsarwithdeps z curl
                      protobuf-lite
        )
# tcmalloc_minimal
install(TARGETS proxy_md_pulsar DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES ${CMAKE_SOURCE_DIR}/coinflex_v2_common/md.fbs DESTINATION ${CMAKE_INSTALL_PREFIX})

#add_custom_command(TARGET proxy_md_lws POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#          ${CMAKE_SOURCE_DIR}/coinflex_v2_common/md.fbs
#          $<TARGET_FILE_DIR:proxy_md_lws>
#        )
