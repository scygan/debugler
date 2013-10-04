include(defines.pri)

TEMPLATE = subdirs

CONFIG += ordered recursive

CONFIG-=debug_and_release

SUBDIRS = lib tests

OTHER_FILES = TODO \
    COPYING \
    AUTHORS \
    INSTALL \
    README \
    THANKS \
    NEWS

message( "Libraries: " )
message( "found by pkg-config: " $$PKG_LIBS )

isEmpty(BUILDING_STATIC_LIBRARY) {
    message("dynamic library")
} else {
    message("static library")
}

message( "actually used      : " $$ADDITIONAL_LIB_LIBRARIES )
message( "actually used      : " $$ADDITIONAL_LIBRARIES )

contains(COMPILER, msvc) {
    isEmpty(BUILDING_STATIC_LIBRARY) {
        warning("on msvc only static version of the library can be currently built")
    }
}
