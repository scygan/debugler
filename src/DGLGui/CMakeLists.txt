find_package(Qt4 REQUIRED)

set(dglgui_SOURCES 
    dglbreakpointdialog.cpp dglbufferview.cpp  dglconnectdialog.cpp
    dglcontoller.cpp dglfboview.cpp dglframebufferview.cpp
    dglgpuview.cpp dglmainwindow.cpp dglpixelrectangle.cpp
    dglprogramview.cpp dglrundialog.cpp dglshaderview.cpp
    dglstateview.cpp dgltabbedview.cpp dgltextureview.cpp
    dgltraceview.cpp dgltreeview.cpp main.cpp
    )

set(dglgui_HEADERS
    dglbreakpointdialog.h dglbufferview.h dglconnectdialog.h
    dglcontroller.h dglfboview.h dglframebufferview.h
    dglgpuview.h dglmainwindow.h
    dglpixelrectangle.h dglprogramview.h dglrundialog.h
    dglshaderview.h dglshaderviewitem.h dglstateview.h
    dgltabbedview.h dgltextureview.h dgltraceview.h
    dgltreeview.h
    )

set(dglgui_FORMS
    ui/dglbreakpointdialog.ui
    ui/dglconnectdialog.ui
    ui/dglfboviewitem.ui
    ui/dglframebufferviewitem.ui
    ui/dglgpuview.ui
    ui/dglmainwindow.ui
    ui/dglpixelrectangleview.ui
    ui/dglprogramview.ui
    ui/dglrundialog.ui
    ui/dglshaderview.ui
    ui/dglstateview.ui
    ui/dgltextureviewitem.ui
    )

SET(dglgui_RESOURCES dglmainwindow.qrc)

QT4_WRAP_CPP(dglgui_HEADERS_MOC ${dglgui_HEADERS})

QT4_WRAP_UI(dglgui_FORMS_HEADERS ${dglgui_FORMS})

QT4_ADD_RESOURCES(dglgui_RESOURCES_RCC ${dglgui_RESOURCES})

include(${QT_USE_FILE})

include_directories(
    ../external/source-highlight-3.1.7/lib
    ${CMAKE_CURRENT_BINARY_DIR}
    )
add_definitions(${QT_DEFINITIONS})

add_executable(dglgui
    ${dglgui_FORMS_HEADERS}
    ${dglgui_SOURCES}
    ${dglgui_HEADERS_MOC}
    ${dglgui_RESOURCES_RCC}
)

add_dependencies(dglgui Boost)

get_property(Boost_LIBRARIES GLOBAL PROPERTY Boost_LIBRARIES)

target_link_libraries(dglgui ${Boost_LIBRARIES} QHexEdit srchiliteqt dglcommon dglnet ${QT_LIBRARIES})
