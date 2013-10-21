set(dglcommon_SOURCES 
    gl-types.cpp os.cpp ipc.cpp
    )



set(codegen_in ${CMAKE_CURRENT_SOURCE_DIR}/../codegen/input)
set(codegen_out ${CMAKE_CURRENT_BINARY_DIR}/../codegen/)

set(codegen_GENERATED
    ${codegen_out}/nonExtTypedefs.inl
    ${codegen_out}/wrappers.inl
    ${codegen_out}/exporters.inl
    ${codegen_out}/exporters-ext.inl
    ${codegen_out}/functionList.inl
    ${codegen_out}/enum.inl
)
add_custom_command(
    OUTPUT ${codegen_GENERATED}
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/../codegen/codegen.py 
        ${codegen_in} ${codegen_out}
    MAIN_DEPENDENCY ../codegen/input/glext.h
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/../codegen/codegen.py
        ${codegen_in}/GL/GL.h
        ${codegen_in}/GL/glext.h
        ${codegen_in}/GL/wgl-notrace.h
        ${codegen_in}/GL/wgl.h
        ${codegen_in}/GL/wglext.h 
        ${codegen_in}/EGL/egl.h 
        ${codegen_in}/EGL/eglext.h 
		${codegen_in}/GLESv1/gl.h
		${codegen_in}/GLESv1/glext.h
        ${codegen_in}/GLES2/gl2.h
		${codegen_in}/GLES2/gl2ext.h
        ${codegen_in}/GLES3/gl3.h
        ${codegen_in}/GL/glx.h
        ${codegen_in}/GL/glxext.h
    COMMENT "Generating code from GL headers"
    VERBATIM
    )

include_directories(
    ..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    )

add_library(dglcommon
    ${dglcommon_SOURCES}
    ${codegen_GENERATED}
)
set_property(TARGET dglcommon PROPERTY COMPILE_FLAGS "-fPIC")
