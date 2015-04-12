#
#	Makefile for dcmwlm/apps
#



SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

ofstddir = $(top_srcdir)/../ofstd
ofstdinc = -I$(ofstddir)/include
ofstdlibdir = -L$(ofstddir)/libsrc
ofstdlib = -lofstd

oflogdir = $(top_srcdir)/../oflog
ofloginc = -I$(oflogdir)/include
ofloglibdir = -L$(oflogdir)/libsrc
ofloglib = -loflog

dcmdatadir = $(top_srcdir)/../dcmdata
dcmdatainc = -I$(dcmdatadir)/include
dcmdatalibdir = -L$(dcmdatadir)/libsrc
dcmdatalib = -ldcmdata

dcmnetdir = $(top_srcdir)/../dcmnet
dcmnetinc = -I$(dcmnetdir)/include
dcmnetlibdir = -L$(dcmnetdir)/libsrc
dcmnetlib = -ldcmnet

LOCALINCLUDES = $(dcmnetinc) $(dcmdatainc) $(ofloginc) $(ofstdinc)
LIBDIRS = -L$(top_srcdir)/libsrc $(dcmnetlibdir) $(dcmdatalibdir) $(ofloglibdir) $(ofstdlibdir)
LOCALLIBS = -ldcmwlm $(dcmnetlib) $(dcmdatalib) $(ofloglib) $(ofstdlib) $(ZLIBLIBS) $(TCPWRAPPERLIBS)

objs = wlcefs.o wlmscpfs.o
progs = wlmscpfs


all: $(progs)

wlmscpfs: $(objs)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $(objs) $(LOCALLIBS) $(MATHLIBS) $(LIBS)


install: all
	$(configdir)/mkinstalldirs $(DESTDIR)$(bindir)
	for prog in $(progs); do \
		$(INSTALL_PROGRAM) $$prog$(BINEXT) $(DESTDIR)$(bindir) && strip $(DESTDIR)$(bindir)/$$prog$(BINEXT) ;\
	done


clean:
	rm -f $(objs) $(progs) $(TRASH)

distclean:
	rm -f $(objs) $(progs) $(DISTTRASH)


dependencies:
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
