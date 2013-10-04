find_package(Qt4 REQUIRED)

set(srchiliteqt_SOURCES 
    #./lib/srchiliteqt/Qt3TextFormatter.cpp
    #./lib/srchiliteqt/ColorDialog.cpp
    ./lib/srchiliteqt/LanguageComboBox.cpp
    ./lib/srchiliteqt/ParagraphMap.cpp
    ./lib/srchiliteqt/TextEditHighlighted.cpp
    ./lib/srchiliteqt/SourceHighlightExceptionBox.cpp
    #./lib/srchiliteqt/Qt3TextFormatterFactory.cpp
    ./lib/srchiliteqt/QtColorMap.cpp
    ./lib/srchiliteqt/Qt4SourceHighlightStyleGenerator.cpp
    ./lib/srchiliteqt/Qt4TextFormatterFactory.cpp
    ./lib/srchiliteqt/GNUSyntaxHighlighter.cpp
    #./lib/srchiliteqt/MainColorForm.cpp
    #./lib/srchiliteqt/Qt3SyntaxHighlighter.cpp
    ./lib/srchiliteqt/StyleComboBox.cpp
    ./lib/srchiliteqt/OutputFormatComboBox.cpp
    #./lib/srchiliteqt/SourceHighlightSettingsPage.cpp
    ./lib/srchiliteqt/TextFormatter.cpp
    #./lib/srchiliteqt/SourceHighlightSettingsDialog.cpp
    ./lib/srchiliteqt/Qt4SyntaxHighlighter.cpp
    ./lib/srchiliteqt/Qt4TextFormatter.cpp
    ./lib/srchiliteqt/TextFormatterFactory.cpp
    #./lib/srchiliteqt/LanguageElemColorForm.cpp
    )


set(srchiliteqt_HEADERS
    #lib/srchiliteqt/LanguageElemColorForm.h
    #lib/srchiliteqt/LanguageComboBox.h
    #lib/srchiliteqt/TextEditHighlighted.h
    #lib/srchiliteqt/StyleComboBox.h
    #lib/srchiliteqt/ColorDialog.h
    #lib/srchiliteqt/SourceHighlightSettingsDialog.h
    #lib/srchiliteqt/SourceHighlightSettingsPage.h
    #lib/srchiliteqt/MainColorForm.h
    #lib/srchiliteqt/OutputFormatComboBox.h
    )

QT4_WRAP_CPP(srchiliteqt_HEADERS_MOC ${srchiliteqt_HEADERS})

include(${QT_USE_FILE})

add_library(srchiliteqt
    ${srchiliteqt_SOURCES}
    ${srchiliteqt_HEADERS_MOC}
)

target_link_libraries(srchiliteqt source-highlight)
