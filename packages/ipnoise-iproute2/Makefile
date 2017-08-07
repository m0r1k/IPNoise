APPNAME	= 	iproute2
APPDIR 	= 	./iproute2-rc
CFLAGS 	= 	-I$(PWD)/../ -I$(PWD)/../ipnoise-kernel/.files/usr/include/

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= $(APPNAME)/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	iproute2-prepare-lin32		\
			iproute2-platform-changed	\
			iproute2-make-lin32			\
			iproute2-install-lin32		\
			iproute2-rpms				\
			iproute2-clean-lin32

include ../Makefile.mk

deps:
	$(MAKE) -C ../ipnoise-kernel	lin32
	$(MAKE) -C ../ipnoise-kernel	install"

$(APPNAME)/Makefile:
	cd $(APPNAME) \
		&& ./configure

iproute2-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME) MFLAGS="$(CFLAGS)"

iproute2-install-lin32:
	cd $(APPNAME)												\
		&& install -d	-m 0755			$(DESTDIR)/usr/			\
		&& install -d	-m 0755			$(DESTDIR)/usr/sbin/	\
		&& install 		-m 0555 ip/ip	$(DESTDIR)/usr/sbin/ip

iproute2-clean-lin32:
	rm -rf $(DESTDIR)/*
	$(MAKE) -C $(APPNAME) clean

## not used
iproute2-prepare-lin32:
iproute2-platform-changed:
iproute2-rpms:

