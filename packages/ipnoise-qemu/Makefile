APPNAME	= 	qemu
APPDIR 	= 	./qemu-1.7.0

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 win32 arm32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	qemu-prepare			\
			qemu-prepare-lin32		\
			qemu-prepare-win32		\
			qemu-prepare-arm32		\
			qemu-platform-changed	\
			qemu-make				\
			qemu-make-lin32			\
			qemu-make-win32			\
			qemu-make-arm32			\
			qemu-install			\
			qemu-install-lin32		\
			qemu-install-win32		\
			qemu-install-arm32		\
			qemu-rpms				\
			qemu-clean				\
			qemu-clean-lin32		\
			qemu-clean-win32		\
			qemu-clean-arm32

include ../Makefile.mk

qemu-prepare:
	@rm -f $(APPNAME)/config.log

qemu-prepare-lin32: qemu-prepare
	cd $(APPNAME);														\
	PKG_CONFIG_PATH=/usr/lib/pkgconfig/;								\
	export PKG_CONFIG_PATH;												\
	./configure 														\
	    --cross-prefix= 												\
	    --host-cc=gcc 													\
	    --audio-drv-list=  												\
		--enable-virtfs													\
		--disable-debug-tcg												\
		--disable-tcg-interpreter										\
	    --extra-cflags="-I $(PWD)/../" 									\
	    --extra-ldflags=-lrt											\
		--target-list=i386-softmmu										\
		--prefix=/usr/;

qemu-prepare-win32: qemu-prepare
	cd $(APPNAME);														\
	PATH=/usr/i686-w64-mingw32/sys-root/mingw/bin/:$$PATH; 				\
	PKG_CONFIG_PATH=/usr/i686-w64-mingw32/sys-root/mingw/lib/pkgconfig/;\
	export PATH;														\
	export PKG_CONFIG_PATH;												\
	./configure 														\
	    --cross-prefix=i686-w64-mingw32- 								\
	    --host-cc=i686-w64-mingw32-gcc 									\
	    --cpu=i386 														\
	    --audio-drv-list= 												\
		--enable-virtfs													\
		--disable-debug-tcg												\
		--disable-tcg-interpreter										\
	    --extra-cflags="`pkg-config --cflags gthread-2.0` -I $(PWD)/../"	\
	    --extra-ldflags="`pkg-config --libs glib-2.0`"					\
		--target-list=i386-softmmu										\
		--prefix=/;

qemu-prepare-arm32: qemu-prepare
	cd $(APPNAME);														\
	PKG_CONFIG_PATH=/usr/lib/pkgconfig/;								\
	export PKG_CONFIG_PATH;												\
	./configure 														\
	    --cross-prefix=			 										\
	    --host-cc=gcc 													\
	    --cpu=arm   													\
	    --audio-drv-list= 												\
		--enable-virtfs													\
		--disable-debug-tcg												\
		--disable-tcg-interpreter										\
	    --extra-cflags="-I $(PWD)/../" 									\
	    --extra-ldflags=												\
		--target-list=i386-softmmu										\
		--prefix=/usr/													\
		--python=/usr/bin/python2.5;

qemu-make:
	$(MAKE) -C $(APPNAME) -j 4

qemu-make-lin32: qemu-make
qemu-make-win32: qemu-make
qemu-make-arm32: qemu-make

qemu-install:
	$(MAKE) -C $(APPNAME)
	install -m 0755 -d $(DESTDIR)/keymaps
	install -m 0644 $(APPNAME)/pc-bios/keymaps/en-us   									$(DESTDIR)/keymaps
	install -m 0644 $(APPNAME)/pc-bios/keymaps/common   								$(DESTDIR)/keymaps
	install -m 0644 $(APPNAME)/pc-bios/keymaps/modifiers 								$(DESTDIR)/keymaps
	install -m 0644 $(APPNAME)/pc-bios/bios.bin       									$(DESTDIR)
	install -m 0644 $(APPNAME)/pc-bios/linuxboot.bin        							$(DESTDIR)
	install -m 0644 $(APPNAME)/pc-bios/kvmvapic.bin					       				$(DESTDIR)
	install -m 0644 $(APPNAME)/pc-bios/vgabios-cirrus.bin  								$(DESTDIR)

qemu-install-lin32: qemu-install
	install -m 0755 $(APPNAME)/i386-softmmu/qemu-system-i386							$(DESTDIR)/qemu
	#install -m 0755 /usr/lib/libpng15.so.15 $(DESTDIR)/
	#install -m 0755 /usr/lib/libtinfo.so.5  $(DESTDIR)/

qemu-install-win32: qemu-install
	install -m 0755 $(APPNAME)/i386-softmmu/qemu-system-i386.exe						$(DESTDIR)/qemu.exe
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libintl-8.dll             	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libjpeg-62.dll            	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libglib-2.0-0.dll       	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libgthread-2.0-0.dll    	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libpng15-15.dll           	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libssp-0.dll            	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libzip-1.dll              	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/SDL.dll                 	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/zlib1.dll              	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libcurl-4.dll        		$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libcrypto-10.dll       	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libidn-11.dll   	    	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/iconv.dll		   	    	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libssh2-1.dll	   	    	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libssl-10.dll	   	    	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libgnutls-26.dll	     	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcrypt-11.dll	     	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libgpg-error-0.dll	     	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libp11-kit-0.dll	     	$(DESTDIR)
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libtasn1-3.dll		     	$(DESTDIR)

qemu-install-arm32: qemu-install
	install -m 0755 $(APPNAME)/i386-softmmu/qemu-system-i386							$(DESTDIR)/qemu

qemu-rpms:
	rm -f ${RPMS_DIR}/ipnoise-${APPNAME}*
	rm -f ${SRPMS_DIR}/ipnoise-${APPNAME}*
	rpmbuild -bb --target i386 ipnoise-qemu.spec
	rm -rf .rpms
	install -m 0755 -d						${STORE_RPMS_TO}
	cp ${RPMS_DIR}/ipnoise-${APPNAME}* 		${STORE_RPMS_TO}

qemu-clean:
	rm -rf $(DESTDIR)/*
	$(MAKE) -C $(APPNAME) clean

qemu-clean-lin32: qemu-clean
qemu-clean-win32: qemu-clean
qemu-clean-arm32: qemu-clean

## not used
qemu-platform-changed:

