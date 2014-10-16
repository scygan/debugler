set(dglwrapper_SOURCES 
    api-loader.cpp
    debugger.cpp
    gl-context.cpp
	gl-shadowstate.cpp
    gl-objects.cpp
	gl-object-namespace.cpp
    gl-wrappers.cpp
    actions.cpp
    action-manager.cpp
    native-surface.cpp
	dl-intercept.cpp
    display.cpp
	tls.cpp
    gl-utils.cpp
	gl-auxcontext.cpp
    gl-statesetters.cpp
    gl-statequery.cpp
    gl-texunit.cpp
	backtrace.cpp
    globalstate.cpp
	exechook.cpp
    dl.cpp
#dllmain.cpp should be at the end, requires all static ctors to run before dllmain.
    dllmain.cpp
    )
	
if (ANDROID)
	set(wa-soctors_SOURCES wa-soctors.cpp)
endif()

add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/DGLWrapper.h)

add_library(dglwrapper SHARED
#wa-soctors goes at the beginning. This ensures static ctors are called before dtors.
    ${wa-soctors_SOURCES}
	${dglwrapper_SOURCES}
)

if (ANDROID)
	set(libs log)
else()
	set(libs pthread elf rt)
endif()

target_link_libraries(dglwrapper dglnet dglcommon dglinject boost_serialization boost_system boost_iostreams dl zlibstatic ${libs})

install (TARGETS dglwrapper DESTINATION lib)
