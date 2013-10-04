find_package(Qt4 REQUIRED)

set(qhexedit_SOURCES 
    commands.cpp  qhexedit.cpp  qhexedit_p.cpp  xbytearray.cpp
    )


set(qhexedit_HEADERS
    qhexedit.h qhexedit_p.h
    )

QT4_WRAP_CPP(qhexedit_HEADERS_MOC ${qhexedit_HEADERS})

include(${QT_USE_FILE})

add_library(QHexEdit
    ${qhexedit_SOURCES}
    ${qhexedit_HEADERS_MOC}
)
