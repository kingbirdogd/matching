add_executable(xpubxsub xpubxsub.cpp)
target_link_directories(xpubxsub PUBLIC $ENV{HOME}/usr/local/lib $ENV{HOME}/usr/local/lib64)
target_link_libraries(xpubxsub zmq pthread)
install(TARGETS xpubxsub DESTINATION ${CMAKE_INSTALL_PREFIX})