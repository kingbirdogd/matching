add_executable(matching_test matching_test.cpp)
target_include_directories(matching_test PUBLIC ${CMAKE_SOURCE_DIR}/matching/inc)
target_include_directories(matching_test PUBLIC ${CMAKE_SOURCE_DIR}/memory/inc)
target_link_libraries(matching_test engine)