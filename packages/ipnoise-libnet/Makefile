APPNAME	= 	libnet
APPDIR 	= 	./libnet-1.1.2.1-6
CFLAGS 	= 	-I$(PWD)/../

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= $(APPNAME)/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		libnet-prepare-lin32	\
			libnet-make-lin32		\
			libnet-install-lin32	\
			libnet-clean-lin32		\
									\
			libnet-prepare-lin64	\
			libnet-make-lin64		\
			libnet-install-lin64	\
			libnet-clean-lin64		\
									\
			libnet-platform-changed	\
			libnet-rpms

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&& CFLAGS="$(CFLAGS)"									\
			./configure 										\
				--prefix /usr/ 									\
				--exec-prefix /usr/								\
				CFLAGS=-I../../../

## lin32

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

## lin64

libnet-make-lin64: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

libnet-install-lin64:
	cd $(APPNAME)												\
		&& make install

libnet-clean-lin64:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## not used
libnet-prepare-lin32:
libnet-prepare-lin64:
libnet-platform-changed:
libnet-rpms:

