include (../defines.pri)

CONFIG += console
CONFIG += link_prl

TEMPLATE = app

TARGET = qt4_highlighter_example

DEFINES = BASEDIR=\\\"$$PWD/\\\"

SOURCES = qt4_highlighter_example_main.cpp

INCLUDEPATH += . ../../lib ../lib $$INCPATH
DEPENDPATH += ../lib ../../lib .

# make sure that the library we built is read before possible other installed versions
LIBS = -L../lib $$LIBS
LIBS += -l$$LIBRARY_LIB $$PKG_LIBS $$ADDITIONAL_LIBRARIES $$ADDITIONAL_LIB_LIBRARIES

CONFIG(static) {
    PRE_TARGETDEPS += ../lib/lib$${LIBRARY_LIB}.a
}

target.path = /demos/source-highlight-qt
INSTALLS += target
