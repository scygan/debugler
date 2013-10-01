find_package(Qt4 REQUIRED)

set(QT_USE_QTXML TRUE)

set(samples_SOURCES
    main.cpp
    ../../external/glew-1.10.0/src/glew.c
    platform_glfw.cpp sample.cpp
    glutil.cpp
    shader_handling.cpp
    simple.cpp
    texture.cpp
    fbo_msaa.cpp
    )

include_directories(../../external/glfw-3.0.2/include)
include_directories(../../external/glew-1.10.0/include)

add_definitions(${QT_DEFINITIONS} -DGLEW_NO_GLU)

add_executable(samples
    ${samples_SOURCES}
)

target_link_libraries(samples glfw boost_program_options X11 GL)


