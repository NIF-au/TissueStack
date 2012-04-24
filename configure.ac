AC_PREREQ([2.60])

AC_INIT([TissueStack], [1.0], [harald.waxenegger@gmail.com])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/c])
#AC_CONFIG_HEADERS([config.h])

LT_INIT

# Compiler Flags 
CXXFLAGS="$CXXFLAGS -g -Wall -lstdc++ -lm -shared"

# Checks for programs.
AC_PROG_CXX

# Dependencies
#PKG_CHECK_MODULES([DEPS], [... >= 1.6])

# minc header includes (actually not really needed by compiles it seems...)
AC_ARG_WITH([minc2-include-path],
  [AS_HELP_STRING([--with-minc2-include-path],
    [location of the MINC2 headers, defaults to /usr/local/include])],
  [MINC2_CFLAGS="-I$withval"],
  [MINC2_CFLAGS='-I/usr/local/bic/include'])
AC_SUBST([MINC2_CFLAGS])

# Minc shared lib
CXXFLAGS="$CXXFLAGS $MINC2_CFLAGS"
AC_ARG_WITH([minc2-lib-path],
  [AS_HELP_STRING([--with-minc2-lib-path],
    [location of the MINC2 shared libraries, defaults to /usr/local/lib])],
  [MINC2_LIBS="-L$withval -lminc2 -lvolume_io2"],
  [MINC2_LIBS='-lminc2 -lvolume_io2'])
AC_SUBST([MINC2_LIBS])
CXXFLAGS="$CXXFLAGS $MINC2_CFLAGS $MINC2_LIBS"

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CONFIG_FILES([
	Makefile
	src/c/Makefile
])
AC_OUTPUT
