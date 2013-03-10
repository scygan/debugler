set(dglwrapper_SOURCES 
    api-loader.cpp  debugger.cpp
    dllmain.cpp  gl-state.cpp
    gl-wrappers.cpp tracer.cpp
    )

add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/DGLWrapper.h)

add_library(dglwrapper SHARED
    ${dglwrapper_SOURCES}
)
