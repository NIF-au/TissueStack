#
#	Makefile for dcmdata/include
#



SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all:

install:
	$(configdir)/mkinstalldirs $(DESTDIR)$(includedir)/dcmtk/dcmdata
	for file in dcmtk/dcmdata/*.h ; do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(includedir)/dcmtk/dcmdata ;\
	done
	$(configdir)/mkinstalldirs $(DESTDIR)$(includedir)/dcmtk/dcmdata/libi2d
	for file in dcmtk/dcmdata/libi2d/*.h ; do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(includedir)/dcmtk/dcmdata/libi2d ;\
	done


clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
