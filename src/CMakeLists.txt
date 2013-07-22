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


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wno-unknown-pragmas -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wsign-promo -Wstrict-null-sentinel")

# These seems incompatible with Qt:
#  -Wctor-dtor-privacy
#  -Wzero-as-null-pointer-constant
#  -Wsign-conversion
#  -Wswitch-default

# We cannot enable -Wredundant-decls, due to including on Khronos headers. May enable after switch to XML API specs.

# We may consider enabling these, but they are rather hard.
# -Wundef -Wold-style-cast -Wmissing-declarations -Woverloaded-virtual -Wstrict-overflow=5

# Unsupported by current version of compiler:
# -Wuseless-cast

add_subdirectory(external/boost)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
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
