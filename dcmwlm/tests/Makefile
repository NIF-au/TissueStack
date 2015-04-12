#
#	Makefile for dcmwlm/tests
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
dcmnetdir = $(top_srcdir)/../dcmnet

LOCALINCLUDES = -I$(dcmnetdir)/include -I$(dcmdatadir)/include \
	-I$(oflogdir)/include -I$(ofstddir)/include
LIBDIRS = -L$(top_srcdir)/libsrc -L$(dcmnetdir)/libsrc -L$(dcmdatadir)/libsrc \
	-L$(oflogdir)/libsrc -L$(ofstddir)/libsrc
LOCALLIBS = -ldcmwlm -ldcmnet -ldcmdata -loflog -lofstd $(ZLIBLIBS) $(TCPWRAPPERLIBS)

objs = wltest.o
progs = wltest


all: $(progs)

wltest: $(objs)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $(objs) $(LOCALLIBS) $(MATHLIBS) $(LIBS)


install: all


clean:
	rm -f $(objs) $(progs) $(TRASH)

distclean:
	rm -f $(objs) $(progs) $(DISTTRASH)


dependencies:
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
