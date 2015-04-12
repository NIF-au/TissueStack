#
#	Makefile for oflog/libsrc
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

objs = oflog.o apndimpl.o appender.o config.o consap.o \
	factory.o fileap.o filter.o globinit.o hierarchy.o hierlock.o \
	layout.o logger.o logimpl.o logevent.o loglevel.o loglog.o lloguser.o \
	ndc.o nullap.o objreg.o patlay.o pointer.o property.o \
	rootlog.o sleep.o socketap.o sockbuff.o socket.o strhelp.o \
	syncprims.o syslogap.o threads.o timehelp.o unixsock.o

library = liboflog.$(LIBEXT)


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
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
