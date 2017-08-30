APPNAME	= 	tcpdump
APPDIR 	= 	./tcpdump-4.1.1
CFLAGS 	= 	-I$(PWD)/../ -I$(PWD)/../ipnoise-libpcap/libpcap/
LDFLAGS = 	-L$(PWD)/libpcap/ -lssl

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		tcpdump-prepare-lin32		\
			tcpdump-make-lin32			\
			tcpdump-install-lin32		\
			tcpdump-clean-lin32			\
			tcpdump-prepare-lin64		\
			tcpdump-make-lin64			\
			tcpdump-install-lin64		\
			tcpdump-clean-lin64			\
			tcpdump-platform-changed	\
			tcpdump-rpms				\

include ../Makefile.mk

## lin32

tcpdump-prepare-lin32:
	ln -sf ../ipnoise-libpcap/libpcap .

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&&	CFLAGS="$(CFLAGS)"									\
			LDFLAGS="$(LDFLAGS)"								\
			./configure

tcpdump-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C ../ipnoise-libpcap lin32
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE) -C $(APPNAME)

tcpdump-install-lin32:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/				\
		&& install -m 0555 tcpdump 	$(DESTDIR)/usr/sbin/tcpdump

tcpdump-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## lin64

tcpdump-prepare-lin64:
	ln -sf ../ipnoise-libpcap/libpcap .

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&&	CFLAGS="$(CFLAGS)"									\
			LDFLAGS="$(LDFLAGS)"								\
			./configure

tcpdump-make-lin64: $(APPNAME)/Makefile
	$(MAKE) -C ../ipnoise-libpcap lin64
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE) -C $(APPNAME)

tcpdump-install-lin64:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/				\
		&& install -m 0555 tcpdump 	$(DESTDIR)/usr/sbin/tcpdump

tcpdump-clean-lin64:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## not used
tcpdump-platform-changed:
tcpdump-rpms:

