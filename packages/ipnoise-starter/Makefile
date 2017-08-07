APPNAME	= 	starter
APPDIR 	= 	./starter-rc

STORE_FILES_TO	= .files
ARCHES			= lin32 win32 arm32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	starter-prepare-lin32		\
			starter-prepare-win32		\
			starter-prepare-arm32		\
			starter-platform-changed	\
			starter-make-lin32			\
			starter-make-win32			\
			starter-make-arm32			\
			starter-install-lin32		\
			starter-install-win32		\
			starter-install-arm32		\
			starter-rpms				\
			starter-clean				\
			starter-clean-lin32			\
			starter-clean-win32			\
			starter-clean-arm32

include ../Makefile.mk

starter-make-lin32:
	$(MAKE) -C $(APPNAME) lin32

starter-make-win32:
	$(MAKE) -C $(APPNAME) win32

starter-make-arm32:
	$(MAKE) -C $(APPNAME) arm32

starter-install-win32:
	install -m 0755 $(APPNAME)/ipnoise.exe $(DESTDIR)/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_sjlj-1.dll	$(DESTDIR)/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll 		$(DESTDIR)/

starter-install-lin32:
	install -m 0755 $(APPNAME)/ipnoise $(DESTDIR)/

starter-install-arm32:
	install -m 0755 $(APPNAME)/ipnoise $(DESTDIR)/

starter-clean:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

starter-clean-lin32:	starter-clean
starter-clean-win32:	starter-clean
starter-clean-arm32:	starter-clean

## not used
starter-prepare-lin32:
starter-prepare-win32:
starter-prepare-arm32:
starter-platform-changed:
starter-rpms:

