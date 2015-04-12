#
#	Makefile for dcmjpeg/libsrc
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
dcmimgledir = $(top_srcdir)/../dcmimgle
dcmjpegdir = $(top_srcdir)/../dcmjpeg

LOCALINCLUDES = -I$(ofstddir)/include -I$(dcmdatadir)/include -I$(dcmimgledir)/include \
  -I$(dcmjpegdir)/libijg8 -I$(dcmjpegdir)/libijg12 -I$(dcmjpegdir)/libijg16 -I$(oflogdir)/include
LOCALDEFS =

objs = djutils.o  djencode.o djrplol.o  djrploss.o djcparam.o djeijg8.o djdijg8.o  \
       djcodecd.o djdecbas.o djdecext.o djdecpro.o djdecsps.o djdeclol.o djdecsv1.o \
       djcodece.o djencbas.o djencext.o djencpro.o djencsps.o djenclol.o djencsv1.o \
       djeijg12.o djdijg12.o djeijg16.o djdijg16.o djdecode.o dipijpeg.o ddpiimpl.o
library = libdcmjpeg.$(LIBEXT)


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
