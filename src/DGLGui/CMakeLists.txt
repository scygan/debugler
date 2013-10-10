find_package(Qt4 REQUIRED)

set(QT_USE_QTXML TRUE)

set(dglgui_SOURCES 
    dglbreakpointdialog.cpp dglbufferview.cpp  dglconnectdialog.cpp
    dglcontroller.cpp dglfboview.cpp dglframebufferview.cpp
    dglgpuview.cpp dglmainwindow.cpp dglpixelrectangle.cpp
    dglprogramview.cpp dglrundialog.cpp dglshaderview.cpp
    dglstateview.cpp dgltabbedview.cpp dgltextureview.cpp
    dgltraceview.cpp dgltreeview.cpp dglprocess.cpp
    dglsyntaxhighlight.cpp dglblitterbase.cpp
    dglglsleditor.cpp dglconfigdialog.cpp dgladbinterface.cpp
	dglconnectandroiddialog.cpp
    )

set(dglgui_HEADERS
    dglbreakpointdialog.h dglbufferview.h dglconnectdialog.h
    dglcontroller.h dglfboview.h dglframebufferview.h
    dglgpuview.h dglmainwindow.h
    dglpixelrectangle.h dglprogramview.h dglrundialog.h
    dglshaderview.h dglshaderviewitem.h dglstateview.h
    dgltabbedview.h dgltextureview.h dgltraceview.h
    dgltreeview.h dglsyntaxhighlight.h
    dglconfigdialog.h dglglsleditor.h dglconnectandroiddialog.h
    dglprocess.h dgladbinterface.h #dglblitterbase.h <- no need for Qt moc here.
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
    ui/dglconfigdialog.ui
	ui/dglconnectandroid.ui
	ui/dglconnectandroidadb.ui
    )

SET(dglgui_RESOURCES dglmainwindow.qrc)

QT4_WRAP_CPP(dglgui_HEADERS_MOC ${dglgui_HEADERS})

QT4_WRAP_UI(dglgui_FORMS_HEADERS ${dglgui_FORMS})

QT4_ADD_RESOURCES(dglgui_RESOURCES_RCC ${dglgui_RESOURCES})

add_definitions(-DDGL_VERSION="${DGL_VERSION}")

include(${QT_USE_FILE})

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
add_definitions(${QT_DEFINITIONS})

add_library(dglgui
    ${dglgui_FORMS_HEADERS}
    ${dglgui_SOURCES}
    ${dglgui_HEADERS_MOC}
    ${dglgui_RESOURCES_RCC}
)

add_executable(debugler main.cpp)
target_link_libraries(debugler dglgui QHexEdit dglcommon dglnet ${QT_LIBRARIES} boost_system boost_serialization)

install (TARGETS debugler DESTINATION bin)
