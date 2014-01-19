set(codegen_in ${CMAKE_CURRENT_SOURCE_DIR}/input)
set(codegen_out ${CMAKE_CURRENT_BINARY_DIR}/)

set(codegen_GENERATED
    ${codegen_out}/nonExtTypedefs.inl
    ${codegen_out}/wrappers.inl
    ${codegen_out}/exporters.inl
    ${codegen_out}/exporters-ext.inl
    ${codegen_out}/exporters-android.inl
    ${codegen_out}/functionList.inl
    ${codegen_out}/enum.inl
    
    ${codegen_in}/GL/glx.h
    ${codegen_in}/GL/glxext.h
    ${codegen_in}/GL/wgl.h
    ${codegen_in}/GL/wglext.h
    ${codegen_in}/GLES/gl.h
    ${codegen_in}/GL/glext.h
    ${codegen_in}/GLES2/gl2.h
    ${codegen_in}/GLES2/gl2ext.h
    ${codegen_in}/GLES3/gl3.h 
)
add_custom_command(
    OUTPUT ${codegen_GENERATED}
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/../codegen/codegen.py 
        ${codegen_in} ${codegen_out}
        MAIN_DEPENDENCY ${codegen_in}/gl.xml
        DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/../codegen/codegen.py
        ${codegen_in}/gl.xml
        ${codegen_in}/egl.xml
        ${codegen_in}/glx.xml
        ${codegen_in}/wgl.xml
        ${codegen_in}/../wgl-notrace.xml
        ${codegen_in}/../gl-android.xml
        ${codegen_in}/reg.py
        ${codegen_in}/genheaders.py
        ${codegen_in}/../android-gles1ext.exports
        ${codegen_in}/../android-gles2ext.exports
    COMMENT "Generating code from GL headers"
    VERBATIM
    )


add_custom_target(codegen DEPENDS ${codegen_GENERATED})
