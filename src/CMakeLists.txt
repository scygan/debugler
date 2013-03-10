cmake_minimum_required(VERSION 2.8)

project(debugler)



add_subdirectory(external/boost)

include_directories(
    .
    external
    external/source-highlight-3.1.7/lib/
    external/boost/
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_subdirectory(external/source-highlight-3.1.7)
add_subdirectory(external/srchiliteqt)
add_subdirectory(external/QHexEdit)
add_subdirectory(DGLCommon)
add_subdirectory(DGLNet)
add_subdirectory(DGLGui)
add_subdirectory(DGLWrapper)
