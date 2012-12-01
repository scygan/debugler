# Copyright (C) 2009, 2010 Lorenzo Bettini <http://www.lorenzobettini.it>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

dnl Process this file with autoconf to produce a configure script.
AC_INIT(source-highlight-qt,0.2.3)

# this is requested for the install-data-local of doxygen documentation
AC_PREREQ([2.62])

AC_CONFIG_AUX_DIR([build_aux])
AC_CONFIG_MACRO_DIR([m4])

dnl for automake
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

LIBRARY_NAME=libsource-highlight-qt

AC_SUBST(LIBRARY_NAME)

AC_SUBST(exampledir)
exampledir=$docdir/examples

#shared library versioning
LIBRARY_VERSION=4:0:1
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed


AC_SUBST(LIBRARY_VERSION)
AC_DEFINE_UNQUOTED(LIBRARY_VERSION, "$LIBRARY_VERSION")

AC_ARG_ENABLE( qt4, [  --enable-qt4       enable qt4 library], [AC_ENABLE_QT4="1"] )
AC_SUBST(AC_ENABLE_QT4)

AC_ARG_ENABLE( qt3, [  --enable-qt3       enable qt3 library], [AC_ENABLE_QT3="1"] )
AC_SUBST(AC_ENABLE_QT3)

echo "Qt3 enabled: ${AC_ENABLE_QT3}"
echo "Qt4 enabled: ${AC_ENABLE_QT4}"

AC_LANG(C++)

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_LIBTOOL

# this is for testing purpose and development
AC_PATH_PROGS(VALGRIND, valgrind)
AC_PATH_PROGS(BASH_SHELL, bash sh)

# we will use it for highlighting examples for the documentation
AC_PATH_PROGS(SRCHILITEEXE, source-highlight)

# Checks for libraries.
PKG_CHECK_MODULES(SRCHILITE, [source-highlight >= 3.1.1])
AC_SUBST(SRCHILITE_CFLAGS)
AC_SUBST(SRCHILITE_LIBS)

#BNV_HAVE_QT

#PKG_CHECK_MODULES(QT, [qt-mt])
#AC_SUBST(QT_CFLAGS)
#AC_SUBST(QT_LIBS)

#if test "${AC_ENABLE_QT4}" == "yes"; then
#PKG_CHECK_MODULES(QT4, [QtCore QtGui])
#AC_SUBST(QT4_CFLAGS)
#AC_SUBST(QT4_LIBS)
#fi

# Qt with AutoTroll.
if test "${AC_ENABLE_QT4}" != ""; then
AT_WITH_QT([-qt4])

else if test "${AC_ENABLE_QT3}" != ""; then
AT_WITH_QT([-qt3])

else
AT_WITH_QT

# by default enable Qt4
AC_ENABLE_QT4="1"
AC_SUBST(AC_ENABLE_QT4)
fi
fi

AH_TEMPLATE(LIBRARY_VERSION)

AM_CONDITIONAL(USE_QT4, test "${AC_ENABLE_QT4}" != "")
AM_CONDITIONAL(USE_QT3, test "${AC_ENABLE_QT3}" != "")

# Check for doxygen program.
AC_ARG_WITH([doxygen],
            [AS_HELP_STRING([--with-doxygen],
              [build doxygen documentation for the api of the library])],
            [AC_PROG_TRY_DOXYGEN],
            [with_doxygen=check])

AM_CONDITIONAL([HAVE_DOXYGEN],[test -n "$DOXYGEN"])

if test "${AC_ENABLE_QT4}" != ""; then
AC_DEFINE(USE_QT4, [1], [Whether we use qt4])
fi

if test "${AC_ENABLE_QT3}" != ""; then
AC_DEFINE(USE_QT3, [1], [Whether we use qt3])
fi

AC_CONFIG_FILES([
	Makefile
	source-highlight-qt3.pc
	source-highlight-qt4.pc
	m4/Makefile
	tests/Makefile
	lib/Makefile
	lib/srchiliteqt/Makefile
	lib/srchiliteqt/srchiliteqt.doxyfile
	lib/tests/Makefile
	doc/Makefile
])

AC_CONFIG_FILES([lib/tests/valgrind_test.sh], [chmod +x lib/tests/valgrind_test.sh])
AC_CONFIG_FILES([lib/tests/valgrind_suppressions.sh], [chmod +x lib/tests/valgrind_suppressions.sh])

AC_OUTPUT

echo "
  ($PACKAGE_NAME) version $PACKAGE_VERSION
  Prefix............: $prefix
  C++ Compiler......: $CXX $CXXFLAGS $CPPFLAGS
  Linker............: $LD $LDFLAGS $LIBS
  Source-highlight..: $SRCHILITEEXE
  Doxygen...........: ${DOXYGEN:-NONE or not enabled}
"

if test -z "$DOXYGEN"; then
  echo "-----------------------------------------"
  echo " No Doxygen program found - continuing"
  echo " without Doxygen documentation support."
  echo " (it must be enabled with --with-doxygen)"
  echo "-----------------------------------------"
fi
