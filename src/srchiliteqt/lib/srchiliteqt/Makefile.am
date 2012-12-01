#include $(top_srcdir)/boiler/autotroll.mk

#INCLUDES = $(SRCHILITE_CFLAGS)

AM_CXXFLAGS = $(QT_CXXFLAGS)
AM_CPPFLAGS = $(QT_CPPFLAGS) $(SRCHILITE_CFLAGS)
AM_LDFLAGS  = $(QT_LDFLAGS) -version-info $(LIBRARY_VERSION)

LDADD = $(SRCHILITE_LIBS) $(QT_LIBS)

noinst_LTLIBRARIES = libsource-highlight-qt-common.la 
lib_LTLIBRARIES =

libsource_highlight_qt_common_la_SOURCES = \
	GNUSyntaxHighlighter.cpp \
	TextFormatter.cpp \
	TextFormatterFactory.cpp \
	QtColorMap.cpp

libsource_highlight_qt_common_la_LIBADD = $(LDADD)

h_sources = \
	GNUSyntaxHighlighter.h TextFormatter.h TextFormatterFactory.h QtColorMap.h HighlightStateData.h \
	Qt4SyntaxHighlighter.h Qt4TextFormatter.h Qt4TextFormatterFactory.h \
	Qt3SyntaxHighlighter.h Qt3TextFormatter.h Qt3TextFormatterFactory.h ParagraphMap.h \
	StyleComboBox.h LanguageComboBox.h OutputFormatComboBox.h TextEditHighlighted.h \
	LanguageElemColorForm.h MainColorForm.h ColorDialog.h \
	Qt4SourceHighlightStyleGenerator.h SourceHighlightExceptionBox.h \
	SourceHighlightSettingsPage.h SourceHighlightSettingsDialog.h

if USE_QT4
lib_LTLIBRARIES += libsource-highlight-qt4.la

libsource_highlight_qt4_la_SOURCES = \
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

nodist_libsource_highlight_qt4_la_SOURCES = $(BUILT_SOURCES)

libsource_highlight_qt4_la_LIBADD = $(LDADD) $(builddir)/libsource-highlight-qt-common.la

# this will avoid the generation of the #ifndef in the generated .ui.h file
# which would end up being the same of the corresponding .h file, which would be wrong
# it is safe to use -no-protection, since we include the .ui.h only in the .cpp file
AM_UIC_FLAGS = -no-protection 

BUILT_SOURCES = moc_StyleComboBox.cpp moc_LanguageComboBox.cpp moc_OutputFormatComboBox.cpp \
	moc_TextEditHighlighted.cpp \
	ui_LanguageElemColorForm.h moc_LanguageElemColorForm.cpp \
	ui_MainColorForm.h moc_MainColorForm.cpp \
	ui_ColorDialog.h moc_ColorDialog.cpp \
	ui_SourceHighlightSettingsPage.h moc_SourceHighlightSettingsPage.cpp \
	ui_SourceHighlightSettingsDialog.h moc_SourceHighlightSettingsDialog.cpp

moc_%.cpp: %.h
	$(MOC) $< -o $@

ui_%.h: %.ui
	$(UIC) $(AM_UIC_FLAGS) $< -o $@

endif

if USE_QT3
lib_LTLIBRARIES += libsource-highlight-qt3.la

libsource_highlight_qt3_la_SOURCES = \
	Qt3SyntaxHighlighter.cpp \
	Qt3TextFormatter.cpp \
	Qt3TextFormatterFactory.cpp \
	ParagraphMap.cpp

libsource_highlight_qt3_la_LIBADD = $(LDADD) $(builddir)/libsource-highlight-qt-common.la
endif

EXTRA_DIST = $(h_sources) srchiliteqt.doxyfile.in \
	LanguageElemColorForm.ui MainColorForm.ui ColorDialog.ui \
	SourceHighlightSettingsPage.ui SourceHighlightSettingsDialog.ui \
	srchiliteqt.pro

library_includedir=$(includedir)/srchiliteqt
library_include_HEADERS = $(h_sources)

# for doxygen documentation
if HAVE_DOXYGEN
DOXYFILE=$(builddir)/srchiliteqt.doxyfile

help_dir=$(htmldir)/api

doxygen.stamp: $(DOXYFILE)
	$(DOXYGEN) $(DOXYFLAGS) $<
	echo Timestamp > $@

## This rule requires Autoconf 2.62 or newer.
install-data-local: doxygen.stamp
	@$(NORMAL_INSTALL)
	$(MKDIR_P) $(DESTDIR)$(help_dir)
	$(INSTALL_DATA) $(builddir)/html/* $(DESTDIR)$(help_dir)

## Warning: the following rule assumes that this package is the
## sole owner of files below $(help_dir)!
uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -f $(DESTDIR)$(help_dir)/*

clean-local:
	rm -f $(builddir)/html/*
endif

CLEANFILES = doxygen.stamp $(BUILT_SOURCES)
