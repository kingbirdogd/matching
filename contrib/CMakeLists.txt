include(ExternalProject)

#===========================================================
include_directories(${CMAKE_SOURCE_DIR}/include)
ExternalProject_Add(project_libwebsockets
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND cmake -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_SOURCE_DIR} -DLWS_ROLE_WS=1 .
        BUILD_COMMAND make clean && make -j4
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND make install
        INSTALL_DIR ${CMAKE_SOURCE_DIR}
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libwebsockets"
        )
add_library(websockets STATIC IMPORTED GLOBAL)
ExternalProject_Get_Property(project_libwebsockets install_dir)
set_target_properties(websockets PROPERTIES
        IMPORTED_LOCATION ${install_dir}/lib/libwebsockets.a
        INCLUDE_DIRECTORIES ${install_dir}/include
        )
add_dependencies(websockets project_libwebsockets)

add_subdirectory(tests)