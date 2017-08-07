APPNAME	= fake
CFLAGS 	= 

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= ./ipnoise-fake.spec ./Makefile

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	fake-prepare-lin32		\
			fake-platform-changed	\
			fake-make-lin32			\
			fake-install-lin32		\
			fake-rpms				\
			fake-clean-lin32

include ../Makefile.mk

fake-install-lin32:
	install -m 0755 -d $(DESTDIR)/usr/
	install -m 0755 -d $(DESTDIR)/usr/share/
	install -m 0755 -d $(DESTDIR)/usr/share/ipnoise-fake/
	install -m 0555 ipnoise-fake.spec 	$(DESTDIR)/usr/share/ipnoise-fake/ipnoise-fake.spec
	install -m 0555 Makefile			$(DESTDIR)/usr/share/ipnoise-fake/Makefile

fake-clean-lin32:
	rm -rf $(DESTDIR)/*

fake-prepare-lin32:
fake-platform-changed:
fake-make-lin32:
fake-install-lin32:
fake-rpms:

