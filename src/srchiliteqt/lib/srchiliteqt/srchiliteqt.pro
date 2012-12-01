include(../../defines.pri)

TEMPLATE = lib

TARGET = source-highlight-qt4

# save information about the library:
CONFIG += create_prl

# does not work
#QMAKE_EXT_MOC = .moc.cpp
#QMAKE_EXT_UIC = .ui.h

HEADERS = GNUSyntaxHighlighter.h TextFormatter.h TextFormatterFactory.h QtColorMap.h HighlightStateData.h \
        Qt4SyntaxHighlighter.h Qt4TextFormatter.h Qt4TextFormatterFactory.h \
        ParagraphMap.h \
        StyleComboBox.h LanguageComboBox.h OutputFormatComboBox.h TextEditHighlighted.h \
        LanguageElemColorForm.h MainColorForm.h ColorDialog.h \
        Qt4SourceHighlightStyleGenerator.h SourceHighlightExceptionBox.h \
        SourceHighlightSettingsPage.h SourceHighlightSettingsDialog.h

SOURCES += GNUSyntaxHighlighter.cpp \
        TextFormatter.cpp \
        TextFormatterFactory.cpp \
        QtColorMap.cpp \
        Qt4SyntaxHighlighter.cpp \
        Qt4TextFormatter.cpp \
        Qt4TextFormatterFactory.cpp \
        StyleComboBox.cpp \
        LanguageComboBox.cpp \
        OutputFormatComboBox.cpp \
        TextEditHighlighted.cpp \
        LanguageElemColorForm.cpp \
        MainColorForm.cpp \
        ColorDialog.cpp \
        Qt4SourceHighlightStyleGenerator.cpp \
        SourceHighlightExceptionBox.cpp \
        SourceHighlightSettingsPage.cpp \
        SourceHighlightSettingsDialog.cpp

FORMS += LanguageElemColorForm.ui MainColorForm.ui ColorDialog.ui \
        SourceHighlightSettingsPage.ui SourceHighlightSettingsDialog.ui

DESTDIR = ../../lib

LIBS += $$PKG_LIBS $$ADDITIONAL_LIBRARIES $$ADDITIONAL_LIB_LIBRARIES

target.path = /lib

headers.files = $$HEADERS
headers.path = /include/srchiliteqt

INSTALLS += target headers

#CONFIG += create_pc
#QMAKE_PKGCONFIG_REQUIRES = QtGui
#pkgconfig.files = source-highlight-qt4.pc
#pkgconfig.path = /lib/pkgconfig
#INSTALLS += pkgconfig
