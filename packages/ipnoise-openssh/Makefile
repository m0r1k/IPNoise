APPNAME	= 	openssh
APPDIR 	= 	./openssh-7.6p1
CFLAGS 	= 	-I$(PWD)/../

SOURCES_DIR		= ~/rpmbuild/SOURCES/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		openssh-prepare-lin32		\
			openssh-make-lin32			\
			openssh-install-lin32		\
			openssh-clean-lin32			\
										\
			openssh-prepare-lin64		\
			openssh-make-lin64			\
			openssh-install-lin64		\
			openssh-clean-lin64			\
										\
			openssh-platform-changed	\
			openssh-rpms

include ../Makefile.mk

$(APPNAME)/configure:
	cd $(APPNAME)	\
		&& autoconf

$(APPNAME)/Makefile: $(APPNAME)/configure
	cd $(APPNAME) 												\
		&& ./configure											\
			--prefix=											\
			--exec-prefix=/usr									\
			--sysconfdir=/etc/ssh								\
			CPPFLAGS=-I../../

## lin32

openssh-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

openssh-install-lin32:
	$(MAKE) -C $(APPNAME) install-nokeys DESTDIR=$(DESTDIR)
	install -m 0755 -d $(DESTDIR)/lib
	install -m 0755 -d $(DESTDIR)/lib/systemd
	install -m 0755 -d $(DESTDIR)/lib/systemd/system
	install -m 0755 rootfs/lib/systemd/system/sshd.service \
		$(DESTDIR)/lib/systemd/system/sshd.service

openssh-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		$(MAKE) -C $(APPNAME) clean;		\
	fi

## lin64

openssh-make-lin64: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

openssh-install-lin64:
	$(MAKE) -C $(APPNAME) install-nokeys DESTDIR=$(DESTDIR)
	install -m 0755 -d $(DESTDIR)/lib
	install -m 0755 -d $(DESTDIR)/lib/systemd
	install -m 0755 -d $(DESTDIR)/lib/systemd/system
	install -m 0755 rootfs/lib/systemd/system/sshd.service \
		$(DESTDIR)/lib/systemd/system/sshd.service

openssh-clean-lin64:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		$(MAKE) -C $(APPNAME) clean;		\
	fi

## not used
openssh-prepare-lin32:
openssh-prepare-lin64:
openssh-platform-changed:
openssh-rpms:

