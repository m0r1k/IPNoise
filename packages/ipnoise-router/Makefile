APPNAME	= 	router
APPDIR 	= 	./router-rc
CFLAGS 	= 	-I$(PWD)/../
DEFINES =

ifneq ($(DISABLE_HOSTOS),)
DEFINES += " DISABLE_HOSTOS=$(DISABLE_HOSTOS)"
endif

SOURCES_DIR		= ~/rpmbuild/SOURCES/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		router-prepare-lin32	\
			router-make-lin32		\
			router-install-lin32	\
			router-clean-lin32		\
									\
			router-prepare-lin64	\
			router-make-lin64		\
			router-install-lin64	\
			router-clean-lin64		\
									\
			router-platform-changed	\
			router-rpms				\
			deps

include ../Makefile.mk

deps:
	$(MAKE) -C ../ipnoise-libnet	$(IPNOISE_PACKAGES_TARGET)
	$(MAKE) -C ../ipnoise-libnet	install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C ../ipnoise-libevent	$(IPNOISE_PACKAGES_TARGET)
	$(MAKE) -C ../ipnoise-libevent	install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C ../ipnoise-libfuse	$(IPNOISE_PACKAGES_TARGET)
	$(MAKE) -C ../ipnoise-libfuse	install DESTDIR="$(PWD)/$(STORE_FILES_TO)"

## lin32

router-make-lin32: $(APPNAME)/Makefile deps
	DISABLE_HOSTOS_OLD=`[ -r ".disable_hostos" ] && cat .disable_hostos`;	\
		if [ "$$DISABLE_HOSTOS_OLD" != "$(DISABLE_HOSTOS)" ];then	\
			echo $(DISABLE_HOSTOS) > .disable_hostos;				\
			make clean;	\
		fi
	$(MAKE) -C $(APPNAME) $(DEFINES)

router-install-lin32:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/				\
		&& install -m 0555 router-rc $(DESTDIR)/usr/sbin/ipnoise-router
	install -m 0755 -d $(DESTDIR)/etc
	install -m 0755 -d $(DESTDIR)/etc/ipnoise
	install -m 0755 rootfs/etc/ipnoise/config.xml.example \
		$(DESTDIR)/etc/ipnoise/config.xml.example
	install -m 0755 -d $(DESTDIR)/lib
	install -m 0755 -d $(DESTDIR)/lib/systemd
	install -m 0755 -d $(DESTDIR)/lib/systemd/system
	install -m 0755 rootfs/lib/systemd/system/ipnoise-router.service \
		$(DESTDIR)/lib/systemd/system/ipnoise-router.service

router-clean-lin32:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

## lin64

router-make-lin64: $(APPNAME)/Makefile deps
	DISABLE_HOSTOS_OLD=`[ -r ".disable_hostos" ] && cat .disable_hostos`;	\
		if [ "$$DISABLE_HOSTOS_OLD" != "$(DISABLE_HOSTOS)" ];then	\
			echo $(DISABLE_HOSTOS) > .disable_hostos;				\
			make clean;	\
		fi
	$(MAKE) -C $(APPNAME) $(DEFINES)

router-install-lin64:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/				\
		&& install -m 0555 router-rc $(DESTDIR)/usr/sbin/ipnoise-router
	install -m 0755 -d $(DESTDIR)/etc
	install -m 0755 -d $(DESTDIR)/etc/ipnoise
	install -m 0755 rootfs/etc/ipnoise/config.xml.example \
		$(DESTDIR)/etc/ipnoise/config.xml.example
	install -m 0755 -d $(DESTDIR)/lib
	install -m 0755 -d $(DESTDIR)/lib/systemd
	install -m 0755 -d $(DESTDIR)/lib/systemd/system
	install -m 0755 rootfs/lib/systemd/system/ipnoise-router.service \
		$(DESTDIR)/lib/systemd/system/ipnoise-router.service

router-clean-lin64:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

## not used
router-prepare-lin32:
router-prepare-lin64:
router-platform-changed:
router-rpms:

