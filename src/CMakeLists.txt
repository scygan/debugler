cmake_minimum_required(VERSION 2.8)

project(debugler)


include_directories(
    .
    glheaders
    external
    external/boost/
    external/gtest/include
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_definitions(-std=c++11)

add_subdirectory(external/boost)
add_subdirectory(external/QHexEdit)
add_subdirectory(DGLCommon)
add_subdirectory(DGLNet)
add_subdirectory(DGLGui)
add_subdirectory(DGLWrapper)
add_subdirectory(DGLLoader)
add_subdirectory(Installer)
add_subdirectory(external/gtest)
add_subdirectory(tests)
