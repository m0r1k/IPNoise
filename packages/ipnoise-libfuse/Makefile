APPNAME	= 	libfuse
APPDIR 	= 	./fuse-2.9.3
CFLAGS 	= 	-I$(PWD)/../../

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= $(APPNAME)/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	libfuse-prepare-lin32		\
			libfuse-platform-changed	\
			libfuse-make-lin32			\
			libfuse-install-lin32		\
			libfuse-rpms				\
			libfuse-clean-lin32

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&& CFLAGS="$(CFLAGS)"									\
			./configure --enable-lib=iconv

libfuse-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

libfuse-install-lin32:
	cd $(APPNAME)												\
		&& install -m 0755 -d $(DESTDIR)/usr/					\
		&& install -m 0755 -d $(DESTDIR)/usr/lib/				\
		&& cp -f lib/.libs/*  $(DESTDIR)/usr/lib/				\
		&& rm -f $(DESTDIR)/usr/lib/*.o

libfuse-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## not used
libfuse-prepare-lin32:
libfuse-platform-changed:
libfuse-rpms:

