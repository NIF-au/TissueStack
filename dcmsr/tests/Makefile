#
#	Makefile for dcmsr/tests
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

LOCALINCLUDES = -I$(ofstddir)/include -I$(dcmdatadir)/include -I$(oflogdir)/include
LIBDIRS = -L$(top_srcdir)/libsrc -L$(ofstddir)/libsrc -L$(dcmdatadir)/libsrc \
	  -L$(oflogdir)/libsrc
LOCALLIBS = -ldcmsr -ldcmdata -loflog -lofstd $(XMLLIBS) $(ZLIBLIBS)

objs = mkreport.o
progs = mkreport


all: $(progs)

mkreport: mkreport.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ mkreport.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)


install: all


clean:
	rm -f $(objs) $(progs) $(TRASH)

distclean:
	rm -f $(objs) $(progs) $(DISTTRASH)


dependencies:
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
