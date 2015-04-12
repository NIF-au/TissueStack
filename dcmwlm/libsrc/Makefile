#
#	Makefile for dcmwlm/libsrc
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

LOCALINCLUDES = -I$(dcmdatadir)/include -I$(dcmnetdir)/include \
	-I$(oflogdir)/include -I$(ofstddir)/include
LOCALDEFS =

objs = wlds.o wlmactmg.o wldsfs.o wlfsim.o
library = libdcmwlm.$(LIBEXT)


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
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
