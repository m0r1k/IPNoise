APPNAME	= 	kernel
APPDIR 	= 	./linux-4.12.9

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
HASH_PATHS		= ${APPNAME}/ ../ipnoise-common/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	kernel-prepare-lin32		\
			kernel-platform-changed		\
			kernel-make-lin32			\
			kernel-install-lin32		\
			kernel-rpms					\
			kernel-clean-lin32

include ../Makefile.mk

kernel-make-lin32:
	$(MAKE) -C $(APPNAME) oldconfig
	$(MAKE) -C ${APPNAME} KCFLAGS=-I$(PWD)/../

kernel-install-lin32:
	cd ${APPNAME}	\
		&& 	install -m 0755 -d $(DESTDIR)/boot/
	$(MAKE) -C ${APPNAME} install INSTALL_PATH="$(DESTDIR)/boot/"
	$(MAKE) -C ${APPNAME} headers_install ARCH=i386 INSTALL_HDR_PATH=$(DESTDIR)/usr/
	$(MAKE) -C ${APPNAME} modules_install INSTALL_MOD_PATH=$(DESTDIR)/
	find $(DESTDIR) -name build  | xargs -i rm -rf {};
	find $(DESTDIR) -name source | xargs -i rm -rf {};
	## $(MAKE) -C ${APPNAME} headers_install ARCH=i386 INSTALL_HDR_PATH=$(DESTDIR)/usr/include/$(APPNAME)/
	cat target.hash > last_build.hash

kernel-clean:
	rm -rf $(DESTDIR)/*
	if [ -r "${APPNAME}/Makefile" ];then	\
		$(MAKE) -C ${APPNAME} clean;			\
	fi
	rm -f target.hash last_build.hash

kernel-clean-lin32: kernel-clean

## not used
kernel-prepare-lin32:
kernel-platform-changed:
kernel-rpms:

