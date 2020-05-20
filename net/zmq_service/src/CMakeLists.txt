add_library(zmq_service zmq_service.cpp)
target_include_directories(zmq_service PUBLIC ../inc)
target_include_directories(zmq_service PUBLIC ${CMAKE_SOURCE_DIR}/net/zmq_client/inc)