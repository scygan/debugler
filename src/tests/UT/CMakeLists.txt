set(ut_SOURCES
    DGLCommonUT.cpp DGLNetUT.cpp LiveTests.cpp
    main.cpp
    )

add_definitions(-DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(ut
    ${ut_SOURCES}
)

target_link_libraries(ut gtest dglgui dglcommon dglnet boost_serialization boost_system pthread rt)

