cmake_minimum_required(VERSION 2.8)

project(debugler)


include_directories(
    .
    external
    external/boost
    ${CMAKE_CURRENT_BINARY_DIR}
    )

#add_subdirectory(codegen)
add_subdirectory(DGLCommon)
add_subdirectory(DGLNet)
add_subdirectory(DGLGui)
