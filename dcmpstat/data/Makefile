#
#	Makefile for dcmpstat/data
#



SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

files = philips.lut


all:

install:
	$(configdir)/mkinstalldirs $(DESTDIR)$(datadir)
	for file in $(files); do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(datadir) ;\
	done

clean:
	rm -f $(objs) $(progs) $(TRASH)

distclean:
	rm -f $(objs) $(progs) $(DISTTRASH)

dependencies:
