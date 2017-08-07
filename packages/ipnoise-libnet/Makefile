APPNAME	= 	libnet
APPDIR 	= 	./libnet-1.1.2.1-6
CFLAGS 	= 	-I$(PWD)/../

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= $(APPNAME)/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	libnet-prepare-lin32	\
			libnet-platform-changed	\
			libnet-make-lin32		\
			libnet-install-lin32	\
			libnet-rpms				\
			libnet-clean-lin32

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&& CFLAGS="$(CFLAGS)"									\
			./configure 										\
				--prefix $(DESTDIR)/usr/ 						\
				--exec-prefix $(DESTDIR)/usr/

libnet-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

libnet-install-lin32:
	cd $(APPNAME)												\
		&& make install

libnet-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## not used
libnet-prepare-lin32:
libnet-platform-changed:
libnet-rpms:

