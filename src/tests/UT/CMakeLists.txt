find_package(Qt4 REQUIRED)

set(QT_USE_QTXML TRUE)

set(ut_SOURCES
    DGLCommonUT.cpp DGLNetUT.cpp LiveTests.cpp
    main.cpp
    )

set(ut_HEADERS
   LiveProcessWrapper.h
   )

QT4_WRAP_CPP(ut_HEADERS_MOC ${ut_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(ut
    ${ut_SOURCES}
    ${ut_HEADERS_MOC}
)

target_link_libraries(ut gtest dglgui dglcommon dglnet boost_serialization boost_system pthread rt ${QT_LIBRARIES})

