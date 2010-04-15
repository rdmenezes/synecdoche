## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

datadir = $(prefix)/share/locale
catalog_name=BOINC-Manager

locale_dirs = \
	ar \
	be \
	bg \
	ca \
	cs \
	da \
	de \
	el \
	en_US \
	es \
	fi \
	fr \
	hr \
	hu \
	it \
	ja \
	ko_KR \
	lt \
	lv \
	nl \
	pl \
	pt \
	pt_BR \
	ro \
	ru \
	sk \
	sv_SE \
	zh_CN \
	zh_TW

# The following code needs GNU Make

%.mo: %/$(catalog_name).po
	mkdir -p `dirname $@`
	msgfmt -v $< -o $@

install-exec-hook: $(patsubst %,%.mo,$(locale_dirs))
	mydir=$(DESTDIR)$(datadir) ;\
	for lang in $(locale_dirs) ; do \
	  $(INSTALL) -d $${mydir}/$${lang}/LC_MESSAGES ; \
	  $(INSTALL) $${lang}.mo $${mydir}/$${lang}/LC_MESSAGES/$(catalog_name).mo; \
	done

