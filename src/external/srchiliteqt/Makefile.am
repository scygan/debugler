EXTRA_DIST = TODO.txt qt3_notes.txt source-highlight-qt.pro defines.pri THANKS

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = m4 lib tests doc

pkgconfigdir = $(libdir)/pkgconfig

if USE_QT4

pkgconfig_DATA = source-highlight-qt4.pc

else

pkgconfig_DATA = source-highlight-qt3.pc

endif

DISTCHECK_CONFIGURE_FLAGS=--with-doxygen --enable-qt4