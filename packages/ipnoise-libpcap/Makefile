APPNAME	= 	libpcap
APPDIR 	= 	./libpcap-1.1.1
CFLAGS 	= 	-I$(PWD)/

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		libpcap-prepare-lin32		\
			libpcap-make-lin32			\
			libpcap-install-lin32		\
			libpcap-clean-lin32			\
										\
			libpcap-prepare-lin64		\
			libpcap-make-lin64			\
			libpcap-install-lin64		\
			libpcap-clean-lin64			\
										\
			libpcap-platform-changed	\
			libpcap-rpms

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&& CFLAGS="$(CFLAGS)"									\
			./configure

## lin32

libpcap-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

libpcap-install-lin32:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/lib/				\
		&& install -m 0755 libpcap.so.1.1.1	$(DESTDIR)/usr/lib/libpcap.so.1.1.1	\
		&& install -m 0755 libpcap.a        $(DESTDIR)/usr/lib/libpcap.a

libpcap-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## lin64

libpcap-make-lin64: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

libpcap-install-lin64:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/lib/				\
		&& install -m 0755 libpcap.so.1.1.1	$(DESTDIR)/usr/lib/libpcap.so.1.1.1	\
		&& install -m 0755 libpcap.a        $(DESTDIR)/usr/lib/libpcap.a

libpcap-clean-lin64:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## not used
libpcap-prepare-lin32:
libpcap-prepare-lin64:
libpcap-platform-changed:
libpcap-rpms:

