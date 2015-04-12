#
#	Makefile for dcmsign/libsrc
#



SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

ofstddir = $(top_srcdir)/../ofstd
oflogdir = $(top_srcdir)/../oflog
dcmdatadir = $(top_srcdir)/../dcmdata

LOCALINCLUDES = -I$(ofstddir)/include -I$(oflogdir)/include -I$(dcmdatadir)/include
LOCALDEFS =

objs = dcsignat.o sicert.o sidsa.o simd5.o siprivat.o sirsa.o sisprof.o \
	siautopr.o sicreapr.o simaccon.o sinullpr.o siripemd.o sisha1.o \
	sitypes.o sicertvf.o sibrsapr.o
library = libdcmdsig.$(LIBEXT)


all: $(library)

install: $(library)
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
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc > $(DEP)

include $(DEP)
