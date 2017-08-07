APPNAME	= 	neuron
APPDIR 	= 	./neuron-rc
CFLAGS 	= 	-I$(PWD)/../
DEFINES =

RPMS_DIR 		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
SOURCES_DIR		= ~/rpmbuild/SOURCES/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	neuron-prepare-lin32	\
			neuron-platform-changed	\
			neuron-make-lin32		\
			neuron-install-lin32	\
			neuron-rpms				\
			neuron-clean-lin32		\
			deps

include Makefile.mk

deps:
	$(MAKE) -C ../ipnoise-libnet	lin32
	$(MAKE) -C ../ipnoise-libnet	install

neuron-make-lin32: $(APPNAME)/Makefile deps
	$(MAKE) -C $(APPNAME) $(DEFINES)

neuron-install-lin32:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/				\
		&& install -m 0555 neuron-rc $(DESTDIR)/usr/sbin/ipnoise-neuron
	install -m 0755 -d $(DESTDIR)/lib
	install -m 0755 -d $(DESTDIR)/lib/systemd
	install -m 0755 -d $(DESTDIR)/lib/systemd/system
	install -m 0755 rootfs/lib/systemd/system/ipnoise-neuron.service \
		$(DESTDIR)/lib/systemd/system/ipnoise-neuron.service

neuron-clean-lin32:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

## not used
neuron-prepare-lin32:
neuron-platform-changed:
neuron-rpms:

