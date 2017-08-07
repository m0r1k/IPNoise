APPNAME	= 	thttpd
APPDIR 	= 	./thttpd-2.25b
CFLAGS 	=
LDFLAGS =

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	thttpd-prepare-lin32	\
			thttpd-platform-changed	\
			thttpd-make-lin32		\
			thttpd-install-lin32	\
			thttpd-rpms				\
			thttpd-clean-lin32

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) 												\
		&&	CFLAGS="$(CFLAGS)"									\
			LDFLAGS="$(LDFLAGS)"								\
			./configure

thttpd-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

thttpd-install-lin32:
	cd $(APPNAME)													\
		&& install -m 0755 -d $(DESTDIR)/etc/						\
		&& install -m 0755 -d $(DESTDIR)/etc/init.d/				\
		&& install -m 0755 -d $(DESTDIR)/var/						\
		&& install -m 0755 -d $(DESTDIR)/var/lib/					\
		&& install -m 0755 -d $(DESTDIR)/var/lib/thttpd/			\
		&& install -m 0755 -d $(DESTDIR)/var/lib/thttpd/cgi-bin/	\
		&& install -m 0755 -d $(DESTDIR)/var/run/thttpd/			\
		&& install -m 0755 -d $(DESTDIR)/usr/						\
		&& install -m 0755 -d $(DESTDIR)/usr/sbin/					\
		&& install -m 0555 thttpd 							$(DESTDIR)/usr/sbin/thttpd		\
		&& install -m 0755 contrib/redhat-rpm/thttpd.init	$(DESTDIR)/etc/init.d/thttpd
	install -m 0644 etc/thttpd.conf 	$(DESTDIR)/etc/thttpd.conf

thttpd-clean-lin32:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

## not used
thttpd-prepare-lin32:
thttpd-platform-changed:
thttpd-rpms:

