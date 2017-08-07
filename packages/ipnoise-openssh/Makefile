APPNAME	= 	openssh
APPDIR 	= 	./openssh-6.0p1
CFLAGS 	= 	-I$(PWD)/../

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
SOURCES_DIR		= ~/rpmbuild/SOURCES/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	openssh-prepare-lin32		\
			openssh-platform-changed	\
			openssh-make-lin32			\
			openssh-install-lin32		\
			openssh-rpms				\
			openssh-clean-lin32

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&& ./configure											\
			--prefix=											\
			--exec-prefix=/usr									\
			--sysconfdir=/etc/ssh								\
			CPPFLAGS=-I../../

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

## not used
openssh-prepare-lin32:
openssh-platform-changed:
openssh-rpms:

