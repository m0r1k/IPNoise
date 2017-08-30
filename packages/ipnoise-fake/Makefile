APPNAME	= fake
CFLAGS 	=

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= ./ipnoise-fake.spec ./Makefile

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		fake-prepare-lin32		\
			fake-make-lin32			\
			fake-install-lin32		\
			fake-clean-lin32		\
									\
			fake-prepare-lin64		\
			fake-make-lin64			\
			fake-install-lin64		\
			fake-clean-lin64		\
									\
			fake-platform-changed	\
			fake-rpms

include ../Makefile.mk

## lin32

fake-install-lin32:
	install -m 0755 -d $(DESTDIR)/usr/
	install -m 0755 -d $(DESTDIR)/usr/share/
	install -m 0755 -d $(DESTDIR)/usr/share/ipnoise-fake/
	install -m 0555 ipnoise-fake.spec 	$(DESTDIR)/usr/share/ipnoise-fake/ipnoise-fake.spec
	install -m 0555 Makefile			$(DESTDIR)/usr/share/ipnoise-fake/Makefile

fake-clean-lin32:
	rm -rf $(DESTDIR)/*

## lin64

fake-install-lin64:
	install -m 0755 -d $(DESTDIR)/usr/
	install -m 0755 -d $(DESTDIR)/usr/share/
	install -m 0755 -d $(DESTDIR)/usr/share/ipnoise-fake/
	install -m 0555 ipnoise-fake.spec 	$(DESTDIR)/usr/share/ipnoise-fake/ipnoise-fake.spec
	install -m 0555 Makefile			$(DESTDIR)/usr/share/ipnoise-fake/Makefile

fake-clean-lin64:
	rm -rf $(DESTDIR)/*

## not used
fake-prepare-lin32:
fake-make-lin32:
fake-install-lin32:
fake-prepare-lin64:
fake-make-lin64:
fake-install-lin64:
fake-platform-changed:
fake-rpms:

