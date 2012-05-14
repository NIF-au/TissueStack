DIRSRC		=	./

SRCS		=	$(DIRSRC)minc_extract_png.c					\
			$(DIRSRC)slice.c						\
			$(DIRSRC)png_creator.c						\
			$(DIRSRC)thread_pool.c

INCLUDE		=	-I/home/oliver/workspace/minc-2.1.10/libsrc2 -I./headers

LIB_PATH	=	-L/home/oliver/workspace/minc-2.1.10/.libs

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpng								\
			-lpthread

NAME		=	yop

FLAGS		=	-g -Wall -Werror -DDEB

<<<<<<< HEAD
all:	
	@echo "\n================== Compilation ===================="
	@gcc $(SRCS) -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS)
=======
pkgdatadir = $(datadir)/tissuestack
pkgincludedir = $(includedir)/tissuestack
pkglibdir = $(libdir)/tissuestack
pkglibexecdir = $(libexecdir)/tissuestack
am__cd = CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd
install_sh_DATA = $(install_sh) -c -m 644
install_sh_PROGRAM = $(install_sh) -c
install_sh_SCRIPT = $(install_sh) -c
INSTALL_HEADER = $(INSTALL_DATA)
transform = $(program_transform_name)
NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_triplet = x86_64-unknown-linux-gnu
host_triplet = x86_64-unknown-linux-gnu
bin_PROGRAMS = $(top_builddir)/bin/TissueStack$(EXEEXT)
subdir = src/c
DIST_COMMON = $(srcdir)/Makefile.am $(srcdir)/Makefile.in
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
am__aclocal_m4_deps = $(top_srcdir)/configure.ac
am__configure_deps = $(am__aclocal_m4_deps) $(CONFIGURE_DEPENDENCIES) \
	$(ACLOCAL_M4)
mkinstalldirs = $(install_sh) -d
CONFIG_CLEAN_FILES =
CONFIG_CLEAN_VPATH_FILES =
am__vpath_adj_setup = srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`;
am__vpath_adj = case $$p in \
    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
    *) f=$$p;; \
  esac;
am__strip_dir = f=`echo $$p | sed -e 's|^.*/||'`;
am__install_max = 40
am__nobase_strip_setup = \
  srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*|]/\\\\&/g'`
am__nobase_strip = \
  for p in $$list; do echo "$$p"; done | sed -e "s|$$srcdirstrip/||"
am__nobase_list = $(am__nobase_strip_setup); \
  for p in $$list; do echo "$$p $$p"; done | \
  sed "s| $$srcdirstrip/| |;"' / .*\//!s/ .*/ ./; s,\( .*\)/[^/]*$$,\1,' | \
  $(AWK) 'BEGIN { files["."] = "" } { files[$$2] = files[$$2] " " $$1; \
    if (++n[$$2] == $(am__install_max)) \
      { print $$2, files[$$2]; n[$$2] = 0; files[$$2] = "" } } \
    END { for (dir in files) print dir, files[dir] }'
am__base_list = \
  sed '$$!N;$$!N;$$!N;$$!N;$$!N;$$!N;$$!N;s/\n/ /g' | \
  sed '$$!N;$$!N;$$!N;$$!N;s/\n/ /g'
am__installdirs = "$(DESTDIR)$(libdir)" "$(DESTDIR)$(bindir)"
LTLIBRARIES = $(lib_LTLIBRARIES)
libTissueStack_la_LIBADD =
am_libTissueStack_la_OBJECTS = TissueStack.lo
libTissueStack_la_OBJECTS = $(am_libTissueStack_la_OBJECTS)
libTissueStack_la_LINK = $(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=link $(CXXLD) $(AM_CXXFLAGS) \
	$(CXXFLAGS) $(libTissueStack_la_LDFLAGS) $(LDFLAGS) -o $@
PROGRAMS = $(bin_PROGRAMS)
am___top_builddir__bin_TissueStack_OBJECTS =  \
	__top_builddir__bin_TissueStack-TissueStack.$(OBJEXT)
__top_builddir__bin_TissueStack_OBJECTS =  \
	$(am___top_builddir__bin_TissueStack_OBJECTS)
__top_builddir__bin_TissueStack_LDADD = $(LDADD)
__top_builddir__bin_TissueStack_LINK = $(LIBTOOL) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link $(CXXLD) \
	$(__top_builddir__bin_TissueStack_CXXFLAGS) $(CXXFLAGS) \
	$(AM_LDFLAGS) $(LDFLAGS) -o $@
am__dirstamp = $(am__leading_dot)dirstamp
DEFAULT_INCLUDES = -I.
depcomp = $(SHELL) $(top_srcdir)/depcomp
am__depfiles_maybe = depfiles
am__mv = mv -f
CXXCOMPILE = $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
LTCXXCOMPILE = $(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) \
	--mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD = $(CXX)
CXXLINK = $(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) \
	--mode=link $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) \
	$(LDFLAGS) -o $@
SOURCES = $(libTissueStack_la_SOURCES) \
	$(__top_builddir__bin_TissueStack_SOURCES)
DIST_SOURCES = $(libTissueStack_la_SOURCES) \
	$(__top_builddir__bin_TissueStack_SOURCES)
ETAGS = etags
CTAGS = ctags
DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)
ACLOCAL = ${SHELL} /opt/workspace/TissueStack/missing --run aclocal-1.11
AMTAR = ${SHELL} /opt/workspace/TissueStack/missing --run tar
AR = ar
AUTOCONF = ${SHELL} /opt/workspace/TissueStack/missing --run autoconf
AUTOHEADER = ${SHELL} /opt/workspace/TissueStack/missing --run autoheader
AUTOMAKE = ${SHELL} /opt/workspace/TissueStack/missing --run automake-1.11
AWK = gawk
CC = gcc
CCDEPMODE = depmode=gcc3
CFLAGS = -g -O2
CPP = gcc -E
CPPFLAGS = 
CXX = g++
CXXCPP = g++ -E
CXXDEPMODE = depmode=gcc3
CXXFLAGS =  -g -Wall -lstdc++ -lm -shared -I/usr/local/bic/include -I/usr/local/bic/include -lminc2 -lvolume_io2
CYGPATH_W = echo
DEFS = -DPACKAGE_NAME=\"TissueStack\" -DPACKAGE_TARNAME=\"tissuestack\" -DPACKAGE_VERSION=\"1.0\" -DPACKAGE_STRING=\"TissueStack\ 1.0\" -DPACKAGE_BUGREPORT=\"harald.waxenegger@gmail.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"tissuestack\" -DVERSION=\"1.0\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\"
DEPDIR = .deps
DLLTOOL = false
DSYMUTIL = 
DUMPBIN = 
ECHO_C = 
ECHO_N = -n
ECHO_T = 
EGREP = /bin/grep -E
EXEEXT = 
FGREP = /bin/grep -F
GREP = /bin/grep
INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
INSTALL_SCRIPT = ${INSTALL}
INSTALL_STRIP_PROGRAM = $(install_sh) -c -s
LD = /usr/bin/ld -m elf_x86_64
LDFLAGS = 
LIBOBJS = 
LIBS = 
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIPO = 
LN_S = ln -s
LTLIBOBJS = 
MAKEINFO = ${SHELL} /opt/workspace/TissueStack/missing --run makeinfo
MANIFEST_TOOL = :
MINC2_CFLAGS = -I/usr/local/bic/include
MINC2_LIBS = -lminc2 -lvolume_io2
MKDIR_P = /bin/mkdir -p
NM = /usr/bin/nm -B
NMEDIT = 
OBJDUMP = objdump
OBJEXT = o
OTOOL = 
OTOOL64 = 
PACKAGE = tissuestack
PACKAGE_BUGREPORT = harald.waxenegger@gmail.com
PACKAGE_NAME = TissueStack
PACKAGE_STRING = TissueStack 1.0
PACKAGE_TARNAME = tissuestack
PACKAGE_URL = 
PACKAGE_VERSION = 1.0
PATH_SEPARATOR = :
RANLIB = ranlib
SED = /bin/sed
SET_MAKE = 
SHELL = /bin/sh
STRIP = strip
VERSION = 1.0
abs_builddir = /opt/workspace/TissueStack/src/c
abs_srcdir = /opt/workspace/TissueStack/src/c
abs_top_builddir = /opt/workspace/TissueStack
abs_top_srcdir = /opt/workspace/TissueStack
ac_ct_AR = ar
ac_ct_CC = gcc
ac_ct_CXX = g++
ac_ct_DUMPBIN = 
am__include = include
am__leading_dot = .
am__quote = 
am__tar = ${AMTAR} chof - "$$tardir"
am__untar = ${AMTAR} xf -
bindir = ${exec_prefix}/bin
build = x86_64-unknown-linux-gnu
build_alias = 
build_cpu = x86_64
build_os = linux-gnu
build_vendor = unknown
builddir = .
datadir = ${datarootdir}
datarootdir = ${prefix}/share
docdir = ${datarootdir}/doc/${PACKAGE_TARNAME}
dvidir = ${docdir}
exec_prefix = ${prefix}
host = x86_64-unknown-linux-gnu
host_alias = 
host_cpu = x86_64
host_os = linux-gnu
host_vendor = unknown
htmldir = ${docdir}
includedir = ${prefix}/include
infodir = ${datarootdir}/info
install_sh = ${SHELL} /opt/workspace/TissueStack/install-sh
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
localedir = ${datarootdir}/locale
localstatedir = ${prefix}/var
mandir = ${datarootdir}/man
mkdir_p = /bin/mkdir -p
oldincludedir = /usr/include
pdfdir = ${docdir}
prefix = /usr/local
program_transform_name = s,x,x,
psdir = ${docdir}
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
srcdir = .
sysconfdir = ${prefix}/etc
target_alias = 
top_build_prefix = ../../
top_builddir = ../..
top_srcdir = ../..
__top_builddir__bin_TissueStack_SOURCES = TissueStack.cpp
__top_builddir__bin_TissueStack_CXXFLAGS = $(AM_CFLAGS)
lib_LTLIBRARIES = libTissueStack.la
libTissueStack_la_SOURCES = TissueStack.cpp
libTissueStack_la_LDFLAGS = -version-info 1:0:0
all: all-am
>>>>>>> d67e96c7113b7e9878ef99f9cc347b572e9027b7

clean:
	@echo "\n==================== Cleaning ====================="
	@rm -rf *~ \#*\# $(NAME) ./png/*

fclean:	clean
	@rm -rf *.o

re:	fclean all

e:	re
	@echo "\n==================== Execution ====================\n\n"
	@./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"

time:	re
	@echo "\n==================== Execution ====================\n\n"
	@time ./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"
