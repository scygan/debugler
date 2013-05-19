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

if (NOT ANDROID)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif()

add_subdirectory(external/boost)
add_subdirectory(DGLCommon)
add_subdirectory(DGLNet)
if (NOT ANDROID)
    add_subdirectory(external/QHexEdit)
    add_subdirectory(DGLGui)
    add_subdirectory(buildtools/cmake/Installer)
    add_subdirectory(buildtools/cmake/DesktopEntry)
    add_subdirectory(external/gtest)
    add_subdirectory(tests)
endif()
add_subdirectory(DGLWrapper)
add_subdirectory(DGLLoader)
