APPNAME	= 	hostfs
APPDIR 	= 	./hostfs-rc
CFLAGS 	= 	-I$(PWD)/../ipnoise-kernel/.files/usr/include/     	\
			-I$(PWD)/../ipnoise-libevent/libevent/include/ 		\
			-I$(PWD)/../ipnoise-libfuse/libfuse/include/

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	hostfs-prepare-lin32		\
			hostfs-platform-changed		\
			hostfs-make-lin32			\
			hostfs-install-lin32		\
			hostfs-rpms					\
			hostfs-clean-lin32			\
			deps

include ../Makefile.mk

deps:
	$(MAKE) -C ../ipnoise-kernel	lin32
	$(MAKE) -C ../ipnoise-kernel	install"
	$(MAKE) -C ../ipnoise-libevent	lin32
	$(MAKE) -C ../ipnoise-libevent	install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C ../ipnoise-libfuse	lin32
	$(MAKE) -C ../ipnoise-libfuse	install DESTDIR="$(PWD)/$(STORE_FILES_TO)"


$(APPNAME)/Makefile:
	$(MAKE) -C $(APPNAME)

hostfs-make-lin32: $(APPNAME)/Makefile deps
	$(MAKE) -C $(APPNAME)

hostfs-install-lin32:
	cd $(APPNAME)		\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/				\
		&& install -m 0755 hostfs-rc $(DESTDIR)/usr/sbin/ipnoise-hostfs
	install -m 0755 -d $(DESTDIR)/lib
	install -m 0755 -d $(DESTDIR)/lib/systemd
	install -m 0755 -d $(DESTDIR)/lib/systemd/system
	install -m 0755 rootfs/lib/systemd/system/ipnoise-hostfs.service	\
		$(DESTDIR)/lib/systemd/system/ipnoise-hostfs.service

hostfs-clean-lin32:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

## not used
hostfs-prepare-lin32:
hostfs-platform-changed:
hostfs-rpms:

