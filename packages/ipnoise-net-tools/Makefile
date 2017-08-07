APPNAME	= 	net-tools
APPDIR 	= 	./net-tools-1.61
CFLAGS 	= 	-I$(PWD)/../

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	net-tools-prepare-lin32		\
			net-tools-platform-changed	\
			net-tools-make-lin32		\
			net-tools-install-lin32		\
			net-tools-rpms				\
			net-tools-clean-lin32

include ../Makefile.mk

net-tools-make-lin32:
	$(MAKE) -C $(APPNAME) EXTRA_CFLAGS="$(CFLAGS)"

net-tools-install-lin32:
	cd $(APPNAME)													\
		&& install -m 0755 -d $(DESTDIR)/usr/bin/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/					\
		&& install -m 0755 hostname		$(DESTDIR)/usr/bin/		\
		&& install -m 0755 ifconfig		$(DESTDIR)/usr/sbin/		\
		&& install -m 0755 route    	$(DESTDIR)/usr/sbin/       \
		&& install -m 0755 rarp     	$(DESTDIR)/usr/sbin/       \
		&& install -m 0755 mii-tool 	$(DESTDIR)/usr/sbin/       \
		&& install -m 0755 netstat  	$(DESTDIR)/usr/sbin/       \
		&& install -m 0755 slattach 	$(DESTDIR)/usr/sbin/       \
		&& install -m 0755 arp      	$(DESTDIR)/usr/sbin/       \
		&& install -m 0755 nameif   	$(DESTDIR)/usr/sbin/		\
 		&& install -m 0755 plipconfig	$(DESTDIR)/usr/sbin/

net-tools-clean-lin32:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

## not used
net-tools-prepare-lin32:
net-tools-platform-changed:
net-tools-rpms:

