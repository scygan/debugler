#INCLUDES = $(SRCHILITE_CFLAGS) -I$(top_srcdir)/lib

# make sure we use the local header files, and not the installed ones

INCLUDES = -I@srcdir@/.. 

AM_CXXFLAGS = $(QT_CXXFLAGS)
AM_CPPFLAGS = $(QT_CPPFLAGS) $(SRCHILITE_CFLAGS)
AM_LDFLAGS  = $(QT_LDFLAGS) -no-install

LDADD = $(SRCHILITE_LIBS) $(QT_LIBS)
#LDADD = $(QT_LIBS)

check_PROGRAMS = test_get_lang_files test_colormap

test_get_lang_files_SOURCES = test_get_lang_files_main.cpp
test_get_lang_files_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt-common.la

test_colormap_SOURCES = test_colormap_main.cpp
test_colormap_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt-common.la

if USE_QT4
check_PROGRAMS += test_qt4_simple test_qt4_formatter_factory test_qt4_syntaxhighlighter

test_qt4_simple_SOURCES = test_qt4_simple_main.cpp
test_qt4_simple_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt4.la

test_qt4_formatter_factory_SOURCES = test_qt4_formatter_factory_main.cpp
test_qt4_formatter_factory_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt4.la

test_qt4_syntaxhighlighter_SOURCES = test_qt4_syntaxhighlighter_main.cpp
test_qt4_syntaxhighlighter_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt4.la
endif

if USE_QT3
check_PROGRAMS += test_qt3_formatter_factory test_qt3_syntaxhighlighter test_paragraphmap

test_qt3_formatter_factory_SOURCES = test_qt3_formatter_factory_main.cpp
test_qt3_formatter_factory_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt3.la

test_qt3_syntaxhighlighter_SOURCES = test_qt3_syntaxhighlighter_main.cpp
test_qt3_syntaxhighlighter_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt3.la

test_paragraphmap_SOURCES = test_paragraphmap_main.cpp
test_paragraphmap_LDADD = $(LDADD) $(builddir)/../srchiliteqt/libsource-highlight-qt3.la
endif

VALGRIND_TESTS = $(check_PROGRAMS)

VALGRINDTEST = $(builddir)/valgrind_test.sh

check-valgrind: $(VALGRIND_TESTS)
	for prog in $(VALGRIND_TESTS); do \
		$(VALGRINDTEST) ./$$prog ; \
	done ;

EXTRA_DIST = valgrind_test.sh.in valgrind_suppressions.sh.in \
	suppressions.supp \
	asserttestexit.h

TESTS = $(check_PROGRAMS)

