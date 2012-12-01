include $(top_srcdir)/boiler/autotroll.mk

# make sure we use the local header files, and not the installed ones
INCLUDES = -I$(top_srcdir)/lib

AM_CXXFLAGS = $(QT_CXXFLAGS)
AM_CPPFLAGS = $(QT_CPPFLAGS) $(SRCHILITE_CFLAGS)
AM_LDFLAGS  = $(QT_LDFLAGS) -no-install
LDADD    = $(SRCHILITE_LIBS) $(QT_LIBS)

if USE_QT4

check_PROGRAMS = \
	qt4_highlighter_example \
	qt4_highlighter_edit \
	qt4_highlighter_view

qt4_highlighter_example_SOURCES = qt4_highlighter_example_main.cpp
qt4_highlighter_example_LDADD = $(LDADD) \
	../lib/srchiliteqt/libsource-highlight-qt4.la
qt4_highlighter_example_CXXFLAGS = -D BASEDIR="\"$(srcdir)/\""

qt4_highlighter_edit_SOURCES = qt4_highlighter_edit_main.cpp
qt4_highlighter_edit_LDADD = $(LDADD) \
	../lib/srchiliteqt/libsource-highlight-qt4.la

qt4_highlighter_view_SOURCES = qt4_highlighter_view_main.cpp
qt4_highlighter_view_LDADD = $(LDADD) \
	../lib/srchiliteqt/libsource-highlight-qt4.la

#BUILT_SOURCES = qt4_highlighter.moc.cpp qt4_highlighter_win.moc.cpp

example_DATA = $(qt4_highlighter_example_SOURCES) \
	$(qt4_highlighter_edit_SOURCES) \
	$(qt4_highlighter_view_SOURCES) \
	$(LANGFILES)
endif

if USE_QT3
check_PROGRAMS = qt3_highlighter_example

qt3_highlighter_example_SOURCES = qt3_highlighter_example_main.cpp
qt3_highlighter_example_LDADD = $(LDADD) \
	../lib/srchiliteqt/libsource-highlight-qt3.la
qt3_highlighter_example_CXXFLAGS = -D BASEDIR="\"$(srcdir)/\""

example_DATA = $(qt3_highlighter_example_SOURCES) $(LANGFILES)

endif

EXTRA_DIST = simple.lang tests.pro

LANGFILES = simple.lang