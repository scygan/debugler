find_package(Qt4 REQUIRED)

set(QT_USE_QTXML TRUE)

set(samples_SOURCES
    main.cpp
    ../../external/glew/src/glew.c
    platform_glfw.cpp sample.cpp
    glutil.cpp
    samples/shader_handling.cpp
	samples/program_handling.cpp
    samples/simple.cpp
    samples/texture2d.cpp
    samples/texture2d_msaa.cpp
    samples/texture2d_array_msaa.cpp
    samples/texture3d.cpp
    samples/fbo_msaa.cpp
    samples/resize.cpp
	samples/sso.cpp
    )

include_directories(../../external/glfw-3.0.2/include)
include_directories(../../external/glew/include)
include_directories(.)

add_definitions(${QT_DEFINITIONS} -DGLEW_NO_GLU)

add_executable(samples
    ${samples_SOURCES}
)

target_link_libraries(samples glfw boost_program_options X11 GL pthread)


