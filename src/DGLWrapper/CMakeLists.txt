set(dglwrapper_SOURCES 
    api-loader.cpp  debugger.cpp
    gl-state.cpp gl-wrappers.cpp
    actions.cpp native-surface.cpp
	dl-intercept.cpp display.cpp
	tls.cpp gl-utils.cpp
	gl-auxcontext.cpp gl-statesetters.cpp
#dllmain.cpp should be at the end, until all static ctors are fixed:
    dllmain.cpp
    )
	
if (ANDROID)
	set(wa-soctors_SOURCES wa-soctors.cpp)
endif()

add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/DGLWrapper.h)

add_library(dglwrapper SHARED
    ${dglwrapper_SOURCES}
	${wa-soctors_SOURCES}
)

if (ANDROID)
	set(libs log)
else()
	set(libs pthread elf rt)
endif()

target_link_libraries(dglwrapper dglnet dglcommon boost_serialization boost_system dl ${libs})

install (TARGETS dglwrapper DESTINATION lib)
