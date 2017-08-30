APPNAME	= 	linphone
APPDIR 	= 	./linphone-3.5.2
CFLAGS 	= 	-I$(PWD)/../

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= $(APPNAME)/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		linphone-prepare-lin32		\
			linphone-make-lin32			\
			linphone-install-lin32		\
			linphone-clean-lin32		\
										\
			linphone-prepare-lin64		\
			linphone-make-lin64			\
			linphone-install-lin64		\
			linphone-clean-lin64		\
										\
			linphone-platform-changed	\
			linphone-rpms				\

include ../Makefile.mk

$(APPNAME)/Makefile:
	cd $(APPNAME) \
		&&	./configure \
			--prefix=$(DESTDIR)/usr;

## lin32

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

## lin64

linphone-make-lin64: $(APPNAME)/Makefile
	$(MAKE) -C $(APPNAME)

linphone-install-lin64:
	$(MAKE) -C $(APPNAME) install
	if [ "$(DESTDIR)" != "$(PWD)/$(STORE_FILES_TO)" ]; then \
		install -m 0755 -d $(DESTDIR);	\
		cp -rp $(PWD)/$(STORE_FILES_TO)/* $(DESTDIR)/;	\
	fi

linphone-clean-lin64:
	rm -rf $(DESTDIR)/*
	$(MAKE) -C $(APPNAME) clean


## not used
linphone-prepare-lin32:
linphone-prepare-lin64:
linphone-platform-changed:
linphone-rpms:

