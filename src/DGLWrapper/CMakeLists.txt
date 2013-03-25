set(dglwrapper_SOURCES 
    api-loader.cpp  debugger.cpp
    gl-state.cpp gl-wrappers.cpp
    tracer.cpp
#dllmain.cpp should be at the end, until all static ctors are fixed:
    dllmain.cpp
    )

add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/DGLWrapper.h)

add_library(dglwrapper SHARED
    ${dglwrapper_SOURCES}
)

target_link_libraries(dglwrapper -Wl,--whole-archive boost_serialization boost_system boost_thread dglcommon dglnet -Wl,--no-whole-archive dl pthread)
