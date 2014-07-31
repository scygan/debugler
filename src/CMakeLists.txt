
cmake_minimum_required(VERSION 2.8)

project(debugler)

file(STRINGS version.inl DGL_VERSION )

add_subdirectory(../tools/build/cmake ${CMAKE_BINARY_DIR}/tools/build/cmake)


if (ARCH EQUAL 32 OR ARCH EQUAL 64)
    set (CMAKE_C_FLAGS "-m${ARCH} ${CMAKE_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "-m${ARCH} ${CMAKE_CXX_FLAGS}")
endif()

include_directories(
    .
    external
    glheaders
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/codegen
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


if (ANDROID) 
	#WA for sloppy static dtors implementation on Android. 
	#For some reason all c++ static dtors in libdglwrapper.so are run twice, leading to crash.
	
	#This helps (propably disables dtors also). 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-use-cxa-atexit")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wno-unknown-pragmas -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wsign-promo -Wstrict-null-sentinel -Woverloaded-virtual")

# These seems incompatible with Qt:
#  -Wctor-dtor-privacy
#  -Wzero-as-null-pointer-constant
#  -Wsign-conversion
#  -Wswitch-default

# We cannot enable -Wredundant-decls, due to including multiple Khronos headers. May enable after switch to XML API specs.

# We may consider enabling these, but they are rather hard.
# -Wold-style-cast -Wmissing-declarations -Wstrict-overflow=5

# Unsupported by current version of compiler:
# -Wuseless-cast

add_subdirectory(external/boost)
if (NOT ANDROID)
    add_subdirectory(external/gtest)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wundef")

add_subdirectory(codegen)
add_subdirectory(DGLCommon)
add_subdirectory(DGLNet)
add_subdirectory(DGLWrapper)
add_subdirectory(DGLInject)
add_subdirectory(DGLLoader)
if (NOT ANDROID)
    add_subdirectory(external/QHexEdit)
    add_subdirectory(DGLGui)
	add_subdirectory(external/glfw-3.0.2)
    add_subdirectory(tests)
else()
	add_subdirectory(DGLAndroidInstaller)
endif()
