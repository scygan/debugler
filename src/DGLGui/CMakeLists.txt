find_package(Qt4 REQUIRED)

set(QT_USE_QTXML TRUE)

set(dglgui_SOURCES 
    dglbreakpointdialog.cpp
    dglbufferview.cpp
    dglcontroller.cpp
    dglfboview.cpp
    dglrenderbufferview.cpp
    dglframebufferview.cpp
    dglgpuview.cpp
    dglmainwindow.cpp
    dglpixelrectangle.cpp
    dglprogramview.cpp
    dglshaderview.cpp
    dglstateview.cpp
    dgltabbedview.cpp
    dgltextureview.cpp
    dgltraceview.cpp
    dgltreeview.cpp
    dglprocess.cpp
    dglsyntaxhighlight.cpp
    dglblitterbase.cpp
    dglglsleditor.cpp
    dglconfigdialog.cpp
    dgladbinterface.cpp
    dgladbdevice.cpp
    dglprepareandroidwizard.cpp
    dglandroidselectdev.cpp
    dglprojectdialog.cpp
    dglproject_base.cpp
    dglproject_runapp.cpp
    dglproject_attach.cpp
    dglproject_android.cpp
    dglbusydialog.cpp
    dglbacktraceview.cpp
    )

set(dglgui_HEADERS
    dglbreakpointdialog.h
    dglbufferview.h
    dglcontroller.h
    dglfboview.h
    dglrenderbufferview.h
    dglframebufferview.h
    dglgpuview.h
    dglmainwindow.h
    dglpixelrectangle.h
    dglprogramview.h
    dglshaderview.h
    dglshaderviewitem.h
    dglstateview.h
    dgltabbedview.h
    dgltextureview.h
    dgltraceview.h
    dgltreeview.h
    dglprocess.h
    dglsyntaxhighlight.h
    #    dglblitterbase.h
    dglglsleditor.h
    dglconfigdialog.h
    dgladbinterface.h
    dgladbdevice.h
    dglprepareandroidwizard.h
    dglandroidselectdev.h
    dglprojectdialog.h
    dglproject_base.h
    dglproject_runapp.h
    dglproject_attach.h
    dglproject_android.h
    dglbusydialog.h
    dglbacktraceview.h
    )

set(dglgui_FORMS
    ui/dglprojectdialog.ui
    ui/dglandroidselectdev.ui
    ui/dglshaderview.ui
    ui/dglprogramview.ui
    ui/dglbreakpointdialog.ui
    ui/dglprojproperties_runapp.ui
    ui/dglfboviewitem.ui
    ui/dglrenderbufferviewitem.ui
    ui/dglconfigdialog.ui
    ui/dglframebufferviewitem.ui
    ui/dglmainwindow.ui
    ui/dgltextureviewitem.ui
    ui/dglgpuview.ui
    ui/dglstateview.ui
    ui/dglconnectandroidadb.ui
    ui/dglprojproperties_android.ui
    ui/dglpixelrectangleview.ui
    ui/dglprojproperties_attach.ui
    ui/dglbacktraceview.h
    )

SET(dglgui_RESOURCES dglmainwindow.qrc)

QT4_WRAP_CPP(dglgui_HEADERS_MOC ${dglgui_HEADERS})

QT4_WRAP_UI(dglgui_FORMS_HEADERS ${dglgui_FORMS})

QT4_ADD_RESOURCES(dglgui_RESOURCES_RCC ${dglgui_RESOURCES})

add_definitions(-DDGL_VERSION="${DGL_VERSION}")
add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/dglgui.h)

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

add_dependencies(dglgui codegen)

add_executable(debugler main.cpp)
target_link_libraries(debugler dglgui QHexEdit dglcommon dglnet ${QT_LIBRARIES} boost_system boost_serialization pthread rt)

install (TARGETS debugler DESTINATION bin)
