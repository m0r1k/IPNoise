APPNAME	= 	xz
APPDIR 	= 	./xz-5.0.4

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64 win32 arm32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		xz-prepare				\
			xz-prepare-lin32		\
			xz-prepare-lin64		\
			xz-prepare-win32		\
			xz-prepare-arm32		\
			xz-platform-changed		\
			xz-make					\
			xz-make-lin32			\
			xz-make-lin64			\
			xz-make-win32			\
			xz-make-arm32			\
			xz-install				\
			xz-install-lin32		\
			xz-install-lin64		\
			xz-install-win32		\
			xz-install-arm32		\
			xz-rpms					\
			xz-clean				\
			xz-clean-lin32			\
			xz-clean-lin64			\
			xz-clean-win32			\
			xz-clean-arm32

include ../Makefile.mk

xz-prepare:
	@rm -f $(APPNAME)/config.log

xz-prepare-lin32: xz-prepare
	if [ ! -r "$(APPNAME)/Makefile" ];then								\
		cd $(APPNAME);													\
			PKG_CONFIG_PATH=/usr/lib/pkgconfig/;						\
			export PKG_CONFIG_PATH;										\
			./configure;												\
	fi

xz-prepare-lin64: xz-prepare
	if [ ! -r "$(APPNAME)/Makefile" ];then								\
		cd $(APPNAME);													\
			PKG_CONFIG_PATH=/usr/lib/pkgconfig/;						\
			export PKG_CONFIG_PATH;										\
			./configure;												\
	fi

xz-prepare-win32: xz-prepare
	if [ ! -r "$(APPNAME)/Makefile" ];then									\
		cd $(APPNAME);														\
		PATH=/usr/i686-w64-mingw32/sys-root/mingw/bin/:$$PATH; 				\
		PKG_CONFIG_PATH=/usr/i686-w64-mingw32/sys-root/mingw/lib/pkgconfig/;\
		export PATH;														\
		export PKG_CONFIG_PATH;												\
		./configure 														\
		    --host=i686-w64-mingw32;										\
	fi

xz-prepare-arm32: xz-prepare
	if [ ! -r "$(APPNAME)/Makefile" ];then									\
		cd $(APPNAME);														\
		PKG_CONFIG_PATH=/usr/lib/pkgconfig/;								\
		export PKG_CONFIG_PATH;												\
		./configure;														\
	fi

xz-make:
	$(MAKE) -C $(APPNAME) -j 4

xz-make-lin32: xz-make
xz-make-lin64: xz-make
xz-make-win32: xz-make
xz-make-arm32: xz-make

xz-install:
	install -m 0644 $(APPNAME)/src/liblzma/.libs/liblzma.a	$(DESTDIR)/

xz-install-lin32: xz-install
xz-install-lin64: xz-install
xz-install-win32: xz-install
xz-install-arm32: xz-install

xz-clean:
	rm -rf $(DESTDIR)/*
	if [ -r "$(MAKE)/Makefile" ];then	\
		$(MAKE) -C $(APPNAME) clean;	\
	fi

xz-clean-lin32: xz-clean
xz-clean-lin64: xz-clean
xz-clean-win32: xz-clean
xz-clean-arm32: xz-clean

xz-platform-changed:
	rm -f  "$(APPNAME)/Makefile"

## not used
xz-rpms:

