add_library(zmq_client zmq_client.cpp)
target_include_directories(zmq_client PUBLIC ../inc)
target_include_directories(zmq_client PUBLIC $ENV{HOME}/usr/local/include)
