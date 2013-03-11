set(dglwrapper_SOURCES 
    api-loader.cpp  debugger.cpp
    dllmain.cpp  gl-state.cpp
    gl-wrappers.cpp tracer.cpp
    )

add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/DGLWrapper.h)

add_library(dglwrapper SHARED
    ${dglwrapper_SOURCES}
)

add_dependencies(dglwrapper Boost)

get_property(Boost_LIBRARIES GLOBAL PROPERTY Boost_LIBRARIES)

target_link_libraries(dglwrapper ${Boost_LIBRARIES} -Wl,--whole-archive dglcommon dglnet -Wl,--no-whole-archive dl)
