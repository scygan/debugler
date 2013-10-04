MAJOR_VERSION = 2
VERSION = $${MAJOR_VERSION}.0.0

isEmpty(NO_PKGCONFIG) {
# handle pkg-config files
for(PKGCONFIG_LIB, $$list(source-highlight)) {
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
    QMAKE_CFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
    #LIBS += $$system(pkg-config --libs $$PKGCONFIG_LIB)
    PKG_LIBS += $$system(pkg-config --libs $$PKGCONFIG_LIB)
}
}

# -------------------------------------------------
# Auto select compiler
# -------------------------------------------------
win32-g++:      COMPILER = mingw
win32-msvc*:    COMPILER = msvc
linux-g++:      COMPILER = gcc

# better to avoid both builds (especially on windows)
CONFIG-=debug_and_release

LIBRARY_NAME = source-highlight-qt4
LIBRARY_LIB = $${LIBRARY_NAME}

CONFIG(static) {
    BUILDING_STATIC_LIBRARY=1
}

contains(COMPILER, mingw) {
    isEmpty(BUILDING_STATIC_LIBRARY) {
        LIBRARY_LIB=$${LIBRARY_NAME}$${MAJOR_VERSION}
    }
}

contains(COMPILER, msvc) {
    isEmpty(BUILDING_STATIC_LIBRARY) {
        CONFIG+=static
    }
}

PKG_SOURCE_HIGHLIGHT = $$find(PKG_LIBS, -lsource-highlight*)

!isEmpty(SOURCE_HIGHLIGHT_LIB) {
    # remove the library found with pkg-config with the one explicitly specified
    PKG_LIBS = $$replace(PKG_LIBS, $$PKG_SOURCE_HIGHLIGHT, )
    ADDITIONAL_LIBRARIES += -l$$SOURCE_HIGHLIGHT_LIB
} else {
    isEmpty(PKG_SOURCE_HIGHLIGHT) {
        ADDITIONAL_LIBRARIES += -lsource-highlight
    }
}

contains(COMPILER, mingw) {
    isEmpty(BUILDING_STATIC_LIBRARY) {
        ADDITIONAL_LIB_LIBRARIES += $${ADDITIONAL_LIBRARIES}
    }
}

PKG_BOOST_REGEX = $$find(PKG_LIBS, -lboost_regex*)

!isEmpty(BOOST_REGEX) {
    # remove the library found with pkg-config with the one explicitly specified
    PKG_LIBS = $$replace(PKG_LIBS, $$PKG_BOOST_REGEX, )
    ADDITIONAL_LIB_LIBRARIES += -l$$BOOST_REGEX
} else {
    isEmpty(PKG_BOOST_REGEX) {
        # default to something
        ADDITIONAL_LIB_LIBRARIES += -lboost_regex
    }
}

