add_library(md md_book.cpp basic_book.cpp md_implied_book.cpp)
target_include_directories(md PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../inc)
target_include_directories(md PUBLIC ${CMAKE_SOURCE_DIR}/matching/order/inc)
target_include_directories(md PUBLIC ${CMAKE_SOURCE_DIR}/implier/inc)


