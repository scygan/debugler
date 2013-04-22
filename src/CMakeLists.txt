cmake_minimum_required(VERSION 2.8)

project(debugler)


include_directories(
    .
    glheaders
    external
    external/source-highlight-3.1.7/lib/
    external/boost/
    ${CMAKE_CURRENT_BINARY_DIR}
    )

add_subdirectory(external/boost)
add_subdirectory(external/source-highlight-3.1.7)
add_subdirectory(external/srchiliteqt)
add_subdirectory(external/QHexEdit)
add_subdirectory(DGLCommon)
add_subdirectory(DGLNet)
add_subdirectory(DGLGui)
add_subdirectory(DGLWrapper)
add_subdirectory(DGLLoader)
add_subdirectory(Installer)
