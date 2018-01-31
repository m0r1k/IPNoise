APPNAME	= 	kernel
APPDIR 	= 	./linux-4.12.9

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
HASH_PATHS		= ${APPNAME}/ ../ipnoise-common/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		kernel-prepare-lin32		\
			kernel-make-lin32			\
			kernel-install-lin32		\
			kernel-clean-lin32			\
										\
			kernel-prepare-lin64		\
			kernel-make-lin64			\
			kernel-install-lin64		\
			kernel-clean-lin64			\
										\
			kernel-platform-changed		\
			kernel-rpms

include ../Makefile.mk

kernel-make-lin32:
	cd $(APPNAME) && cp -f ipnoise.config .config
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

kernel-make-lin64:
	cd $(APPNAME) && cp -f ipnoise.config .config
	$(MAKE) -C $(APPNAME) oldconfig
	$(MAKE) -C ${APPNAME} KCFLAGS=-I$(PWD)/../

kernel-install-lin64:
	cd ${APPNAME}	\
		&& 	install -m 0755 -d $(DESTDIR)/boot/
	$(MAKE) -C ${APPNAME} install         ARCH=x86_64 INSTALL_PATH="$(DESTDIR)/boot/"
	$(MAKE) -C ${APPNAME} headers_install ARCH=x86_64 INSTALL_HDR_PATH=$(DESTDIR)/usr/
	$(MAKE) -C ${APPNAME} modules_install ARCH=x86_64 INSTALL_MOD_PATH=$(DESTDIR)/
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
kernel-clean-lin64: kernel-clean

## not used
kernel-prepare-lin32:
kernel-prepare-lin64:
kernel-platform-changed:
kernel-rpms:

