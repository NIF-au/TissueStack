#
#	Makefile for dcmwlm/data
#



SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

folders = wlistdb wlistdb/OFFIS wlistqry


all:

install:
	$(configdir)/mkinstalldirs $(DESTDIR)$(datadir)
	for folder in $(folders); do \
		$(configdir)/mkinstalldirs $(DESTDIR)$(datadir)/$$folder ;\
		find $$folder -maxdepth 1 -type f -exec $(INSTALL_DATA) {} $(DESTDIR)$(datadir)/$$folder \; ;\
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
