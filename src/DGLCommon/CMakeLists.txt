set(dglcommon_SOURCES 
    gl-types.cpp os.cpp 
    )



set(codegen_in ${CMAKE_CURRENT_SOURCE_DIR}/../codegen/input)
set(codegen_out ${CMAKE_CURRENT_BINARY_DIR}/../codegen/)

set(codegen_GENERATED
    ${codegen_out}/nonExtTypedefs.inl
    ${codegen_out}/wrappers.inl
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
        ${codegen_in}/GL.h
        ${codegen_in}/glext.h
        ${codegen_in}/wgl-notrace.h
        ${codegen_in}/wgl.h
        ${codegen_in}/wglext-partial.h
        ${codegen_in}/wglext.h 
		${codegen_in}/egl.h 
		${codegen_in}/eglext.h 
		${codegen_in}/gl2.h
		${codegen_in}/gl3.h
		${codegen_in}/glx.h
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
