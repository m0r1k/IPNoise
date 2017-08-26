APPNAME	= 	libevent
APPDIR 	= 	./libevent-2.1.8-stable

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= $(APPNAME)/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	libevent-prepare-lin32		\
			libevent-platform-changed	\
			libevent-make-lin32			\
			libevent-install-lin32		\
			libevent-rpms				\
			libevent-clean-lin32

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) \
		&& ./configure 					\
			CPPFLAGS=-I$(PWD)/../		\
			--disable-thread-support

libevent-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

libevent-install-lin32:
	cd $(APPNAME)	\
		&& install -m 0755 -d $(DESTDIR)/usr/				\
		&& install -m 0755 -d $(DESTDIR)/usr/lib/			\
		&& install -m 0755 -d $(DESTDIR)/usr/include/		\
		&& cp -f .libs/* 		$(DESTDIR)/usr/lib/			\
		&& cp -r include/event2	$(DESTDIR)/usr/include/		\
		&& rm -f $(DESTDIR)/usr/lib/*.o

libevent-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## not used
libevent-prepare-lin32:
libevent-platform-changed:
libevent-rpms:

