APPNAME	= 	packer
APPDIR 	= 	./packer-rc

STORE_FILES_TO	= .files
ARCHES			= lin32 lin64 win32 arm32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/ ../ipnoise-xz/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		default							\
			packer-prepare-lin32			\
			packer-prepare-lin64			\
			packer-prepare-win32			\
			packer-prepare-arm32			\
			packer-platform-changed			\
			packer-make-lin32				\
			packer-make-lin64				\
			packer-make-win32				\
			packer-make-arm32				\
			packer-install-lin32			\
			packer-install-lin64			\
			packer-install-win32			\
			packer-install-arm32			\
			packer-rpms						\
			packer-clean

include ../Makefile.mk

packer-prepare:
	install -m 0755 -d "$(PWD)/$(STORE_FILES_TO)"

packer-prepare-lin32: packer-prepare
packer-prepare-lin64: packer-prepare
packer-prepare-win32: packer-prepare
packer-prepare-arm32: packer-prepare

packer-make-lin32:
	$(MAKE) -C ../ipnoise-xz/ lin32
	$(MAKE) -C ../ipnoise-xz/ install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C $(APPNAME) lin32

packer-make-lin64:
	$(MAKE) -C ../ipnoise-xz/ lin64
	$(MAKE) -C ../ipnoise-xz/ install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C $(APPNAME) lin64

packer-make-win32:
	$(MAKE) -C $(APPNAME) win32

packer-make-arm32:
	$(MAKE) -C $(APPNAME) arm32

packer-install-lin32:
	ar rvs $(DESTDIR)/packer.a `find $(APPNAME)/ -name "*.o" | grep -v "main.o"`
	install -m 0755 $(APPNAME)/packer-rc $(DESTDIR)/packer

packer-install-lin64:
	ar rvs $(DESTDIR)/packer.a `find $(APPNAME)/ -name "*.o" | grep -v "main.o"`
	install -m 0755 $(APPNAME)/packer-rc $(DESTDIR)/packer

packer-install-win32:
	i686-w64-mingw32-ar rvs $(DESTDIR)/packer.a `find $(APPNAME)/ -name "*.o" | grep -v "main.o"`
	install -m 0755 $(APPNAME)/packer-rc $(DESTDIR)/packer

packer-install-arm32:
	ar rvs $(DESTDIR)/packer.a `find $(APPNAME)/ -name "*.o" | grep -v "main.o"`
	install -m 0755 $(APPNAME)/packer-rc $(DESTDIR)/packer

packer-clean:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

packer-clean-lin32: packer-clean
packer-clean-lin64: packer-clean
packer-clean-win32: packer-clean
packer-clean-arm32: packer-clean

## not used
packer-platform-changed:
packer-rpms:

