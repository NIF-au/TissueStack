#
#	Makefile for dcmdata/data
#



SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

files = dicom.dic private.dic diconde.dic dcm2xml.dtd dumppat.txt SC.dump VLP.dump


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
