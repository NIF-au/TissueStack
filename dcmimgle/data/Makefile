#
#	Makefile for dcmimgle/data
#



SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

files = camera.lut monitor.lut printer.lut scanner.lut


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
