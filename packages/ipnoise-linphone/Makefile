APPNAME	= 	linphone
APPDIR 	= 	./linphone-3.5.2
CFLAGS 	= 	-I$(PWD)/../

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= $(APPNAME)/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	linphone-prepare-lin32		\
			linphone-platform-changed	\
			linphone-make-lin32			\
			linphone-install-lin32		\
			linphone-rpms				\
			linphone-clean-lin32

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) \
		&&	./configure \
			--prefix=$(DESTDIR)/usr;

linphone-make-lin32: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

linphone-install-lin32:
	$(MAKE) -C $(APPNAME) install
	if [ "$(DESTDIR)" != "$(PWD)/$(STORE_FILES_TO)" ]; then \
		install -m 0755 -d $(DESTDIR);	\
		cp -rp $(PWD)/$(STORE_FILES_TO)/* $(DESTDIR)/;	\
	fi

linphone-clean-lin32:
	rm -rf $(DESTDIR)/*
	$(MAKE) -C $(APPNAME) clean

## not used
linphone-prepare-lin32:
linphone-platform-changed:
linphone-rpms:

