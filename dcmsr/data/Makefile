#
#	Makefile for dcmsr/data
#



SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

files = dsr2xml.xsd report.css reportx.css


all:

install:
	$(configdir)/mkinstalldirs $(DESTDIR)$(datadir)
	for file in $(files); do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(datadir) ;\
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
