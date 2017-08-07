APPNAME	= 	transport
APPDIR 	= 	./transport-rc
CFLAGS 	= 	-I$(PWD)/../ipnoise-kernel/.files/usr/include/ \
			-I$(PWD)/../ipnoise-libevent/libevent/include/

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	transport-prepare-lin32		\
			transport-platform-changed	\
			transport-make-lin32		\
			transport-install-lin32		\
			transport-rpms				\
			transport-clean-lin32		\
			deps

include ../Makefile.mk

deps:
	$(MAKE) -C ../ipnoise-kernel	lin32
	$(MAKE) -C ../ipnoise-kernel	install
	$(MAKE) -C ../ipnoise-libnet	lin32
	$(MAKE) -C ../ipnoise-libnet	install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C ../ipnoise-libevent	lin32
	$(MAKE) -C ../ipnoise-libevent	install DESTDIR="$(PWD)/$(STORE_FILES_TO)"

$(APPNAME)/Makefile:
	$(MAKE) -C $(APPNAME)

transport-make-lin32: $(APPNAME)/Makefile deps
	$(MAKE) -C $(APPNAME)

transport-install-lin32:
	cd $(APPNAME)		\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/				\
		&& install -m 0755 transport-rc $(DESTDIR)/usr/sbin/ipnoise-transport-tcp-v4	\
		&& install -m 0755 transport-rc $(DESTDIR)/usr/sbin/ipnoise-transport-udp-v4
	install -m 0755 -d $(DESTDIR)/lib
	install -m 0755 -d $(DESTDIR)/lib/systemd
	install -m 0755 -d $(DESTDIR)/lib/systemd/system
	install -m 0755 rootfs/lib/systemd/system/ipnoise-transport-tcp-v4.service	\
		$(DESTDIR)/lib/systemd/system/ipnoise-transport-tcp-v4.service
	install -m 0755 rootfs/lib/systemd/system/ipnoise-transport-udp-v4.service \
		$(DESTDIR)/lib/systemd/system/ipnoise-transport-udp-v4.service

transport-clean-lin32:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

## not used
transport-prepare-lin32:
transport-platform-changed:
transport-rpms:

