#
#	Makefile for dcmjpls/libcharls
#



SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

ofstddir = $(top_srcdir)/../ofstd

LOCALINCLUDES = -I$(ofstddir)/include
LOCALDEFS =

objs = header.o intrface.o jpegls.o

library = libcharls.$(LIBEXT)

all: $(library)

install: all
	$(configdir)/mkinstalldirs $(DESTDIR)$(libdir)
	$(INSTALL_DATA) $(library) $(DESTDIR)$(libdir)/$(library)
	$(RANLIB) $(DESTDIR)$(libdir)/$(library)

$(library): $(objs)
	$(AR) $(ARFLAGS) $@ $(objs)
	$(RANLIB) $@

clean:
	rm -f $(objs) $(library) $(TRASH)

distclean:
	rm -f $(objs) $(library) $(DISTTRASH)


dependencies:
	$(CC) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
