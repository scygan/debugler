set(dglwrapper_SOURCES 
    api-loader.cpp  debugger.cpp
    gl-state.cpp gl-wrappers.cpp
    actions.cpp dl-intercept.cpp
#dllmain.cpp should be at the end, until all static ctors are fixed:
    dllmain.cpp
    )

if (ANDROID)
	include_directories(../external/libelf-0.8.13)
	include_directories(../external/libelf-0.8.13/libelf)
endif()
	
add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/DGLWrapper.h)

add_library(dglwrapper SHARED
    ${dglwrapper_SOURCES}
)

if (ANDROID)
	set(linux_libs, "")
else()
	set(linux_libs, pthread elf)
endif()

target_link_libraries(dglwrapper dglnet dglcommon boost_serialization boost_system boost_thread dl ${linux_libs})

install (TARGETS dglwrapper DESTINATION lib)
