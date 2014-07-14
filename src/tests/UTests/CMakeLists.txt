find_package(Qt4 REQUIRED)

set(QT_USE_QTXML TRUE)

set(utests_SOURCES
    DGLCommonUT.cpp DGLNetUT.cpp LiveTests.cpp DGLGui.cpp
    main.cpp
    )

set(utests_HEADERS
   LiveProcessWrapper.h
   )

QT4_WRAP_CPP(utests_HEADERS_MOC ${utests_HEADERS})

SET(utests_RESOURCES ../../DGLGui/dglmainwindow.qrc)
QT4_ADD_RESOURCES(utests_RESOURCES_RCC ${utests_RESOURCES})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(utests
    ${utests_SOURCES}
    ${utests_HEADERS_MOC}
    ${utests_RESOURCES_RCC}
)

target_link_libraries(utests gtest dglgui dglcommon dglnet boost_serialization boost_system pthread z rt ${QT_LIBRARIES})

