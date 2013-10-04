set(source-highlight_SOURCES 
    lib/srchilite/delimitedlangelem.cpp
    lib/srchilite/fileutil.cc
    lib/srchilite/formatter.cpp
    lib/srchilite/formattermanager.cpp
    lib/srchilite/highlightbuilderexception.cpp
    lib/srchilite/highlightrule.cpp
    lib/srchilite/highlightrulefactory.cpp
    lib/srchilite/highlightstate.cpp
    lib/srchilite/highlightstatebuilder.cpp
    lib/srchilite/highlighttoken.cpp
    lib/srchilite/instances.cpp
    lib/srchilite/ioexception.cpp
    lib/srchilite/langdefmanager.cpp
    lib/srchilite/langdefparser.cc
    lib/srchilite/langdefscanner.cc
    lib/srchilite/langelem.cpp
    lib/srchilite/langelems.cpp
    lib/srchilite/langelemsprinter.cpp
    lib/srchilite/langmap.cpp
    lib/srchilite/namedsubexpslangelem.cpp
    lib/srchilite/parserexception.cpp
    lib/srchilite/regexhighlightrule.cpp
    lib/srchilite/regexpreprocessor.cpp
    lib/srchilite/regexrulefactory.cpp
    lib/srchilite/settings.cpp
    lib/srchilite/sourcefilehighlighter.cpp
    lib/srchilite/sourcehighlighter.cpp
    lib/srchilite/statelangelem.cpp
    lib/srchilite/statestartlangelem.cpp
    lib/srchilite/stringdef.cpp
    lib/srchilite/stringlistlangelem.cpp
    lib/srchilite/stringtable.cpp
    lib/srchilite/stylecssparser.cc
    lib/srchilite/stylecssscanner.cc
    lib/srchilite/stylefileparser.cpp
    lib/srchilite/styleparser.cc
    lib/srchilite/stylescanner.cc
    lib/srchilite/utils.cpp
    lib/srchilite/vardefinitions.cpp
    lib/srchilite/verbosity.cpp
    )
	
#these one will cause loading of highlight definition through file interface
#defined in GUI, reading from embedded resources, rather than files
add_definitions(-DABSOLUTEDATADIR=":/highlight/" -include ${CMAKE_CURRENT_SOURCE_DIR}/../../DGLGui/dglresourcefs.h)

add_library(source-highlight
    ${source-highlight_SOURCES}
)


