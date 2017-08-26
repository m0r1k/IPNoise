APPNAME	= 	squid
APPDIR 	= 	./squid-3.5.27
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

.PHONY =	squid-prepare-lin32		\
			squid-platform-changed	\
			squid-make-lin32		\
			squid-install-lin32		\
			squid-rpms				\
			squid-clean-lin32		\
			squid-clean

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&& ./configure											\
			--prefix=											\
			--enable-storeio=									\
			--exec-prefix=/usr									\
				CFLAGS=-I../../../								\
				CPPFLAGS=-I../../../

##			--disable-ipv6										\

squid-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

squid-install-lin32:
	make -C $(APPNAME) install DESTDIR=$(DESTDIR)
	rm -rf $(DESTDIR)/share
	rm -rf $(DESTDIR)/include
	install -m 0755 -d $(DESTDIR)/etc/
	install -m 0755 -d $(DESTDIR)/lib/
	install -m 0755 -d $(DESTDIR)/lib/systemd/
	install -m 0755 -d $(DESTDIR)/lib/systemd/system/
	install -m 0755 root/etc/squid.conf	$(DESTDIR)/etc/squid.conf
	install -m 0755 root/lib/systemd/system/ipnoise-squid.service $(DESTDIR)/lib/systemd/system/ipnoise-squid.service

squid-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then		\
		make -C $(APPNAME) clean || exit 1;		\
	fi
	rm -f "$(APPNAME)/Makefile"

squid-clean: squid-clean-lin32

## not used
squid-prepare-lin32:
squid-platform-changed:
squid-rpms:

