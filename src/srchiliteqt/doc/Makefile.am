# Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>

info_TEXINFOS = source-highlight-qt.texinfo

source_highlight_qt_TEXINFOS = $(GENERATEDINFOS)

AM_MAKEINFOHTMLFLAGS=--no-split
# for including generated texinfo files
AM_MAKEINFOFLAGS=-I$(srcdir)

doc_DATA = source-highlight-qt.html

SRCHILITETEXINFO = $(SRCHILITEEXE) -f texinfo --output-dir=$(top_builddir)/doc/ --style-file=texinfo.style

# texinfo files generated starting from the examples
GENERATEDINFOS = qt4_highlighter_example_main.cpp.texinfo \
	qt4_highlighter_edit_main.cpp.texinfo \
	simpleusage.cpp.texinfo comboboxes.cpp.texinfo \
	colordialog.cpp.texinfo textedithighlighted_exceptions.cpp.texinfo \
	settingsdialog.cpp.texinfo

EXTRA_DIST = $(GENERATEDINFOS) source-highlight-qt.html \
	simpleusage.cpp comboboxes.cpp colordialog.cpp \
	settingsdialog.cpp

qt4_highlighter_example_main.cpp.texinfo: $(top_srcdir)/tests/qt4_highlighter_example_main.cpp
	$(SRCHILITETEXINFO) $(top_srcdir)/tests/qt4_highlighter_example_main.cpp

qt4_highlighter_edit_main.cpp.texinfo: $(top_srcdir)/tests/qt4_highlighter_edit_main.cpp
	$(SRCHILITETEXINFO) $(top_srcdir)/tests/qt4_highlighter_edit_main.cpp --regex-range="// TEXINFOINCLUDE"

simpleusage.cpp.texinfo: $(srcdir)/simpleusage.cpp
	$(SRCHILITETEXINFO) $(srcdir)/simpleusage.cpp

comboboxes.cpp.texinfo: $(srcdir)/comboboxes.cpp
	$(SRCHILITETEXINFO) $(srcdir)/comboboxes.cpp

colordialog.cpp.texinfo: $(srcdir)/colordialog.cpp
	$(SRCHILITETEXINFO) $(srcdir)/colordialog.cpp

settingsdialog.cpp.texinfo: $(srcdir)/settingsdialog.cpp
	$(SRCHILITETEXINFO) $(srcdir)/settingsdialog.cpp

textedithighlighted_exceptions.cpp.texinfo: $(top_srcdir)/lib/srchiliteqt/TextEditHighlighted.cpp
	$(SRCHILITETEXINFO) -i $(top_srcdir)/lib/srchiliteqt/TextEditHighlighted.cpp -o textedithighlighted_exceptions.cpp.texinfo --regex-range="// TEXINFOINCLUDE"