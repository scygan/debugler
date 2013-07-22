cmake_minimum_required(VERSION 2.8)

if (ARCH EQUAL 32 OR ARCH EQUAL 64)
    set (CMAKE_C_FLAGS "-m${ARCH}")
    set (CMAKE_CXX_FLAGS "-m${ARCH}")
endif()

project(debugler)


if(NOT DEFINED DGL_VERSION)
    set(DGL_VERSION 0.0.0)
endif()


include_directories(
    .
    glheaders
    external
    ${CMAKE_CURRENT_BINARY_DIR}
    )

include_directories(SYSTEM
    external/boost/
    external/gtest/include
)


if (NOT ANDROID)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic -Wall -Wno-unknown-pragmas")# -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wzero-as-null-pointer-constant") #-Wuseless-cast")

add_subdirectory(external/boost)
add_subdirectory(DGLCommon)
add_subdirectory(DGLNet)
if (NOT ANDROID)
    add_subdirectory(external/QHexEdit)
    add_subdirectory(DGLGui)
    add_subdirectory(buildtools/cmake)
    add_subdirectory(external/gtest)
    add_subdirectory(tests)
endif()
add_subdirectory(DGLWrapper)
add_subdirectory(DGLLoader)
