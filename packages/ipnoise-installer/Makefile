APPNAME	= 	installer
APPDIR 	= 	./installer-rc

STORE_FILES_TO	= .files
ARCHES			= lin32 win32 arm32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/ ../ipnoise-packer/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	installer-prepare-lin32			\
			installer-prepare-win32			\
			installer-prepare-arm32			\
			installer-platform-changed		\
			installer-make-lin32			\
			installer-make-win32			\
			installer-make-arm32			\
			installer-install				\
			installer-install-lin32			\
			installer-install-win32			\
			installer-install-arm32			\
			installer-rpms					\
			installer-clean

include ../Makefile.mk

installer-prepare:
	install -m 0755 -d "$(PWD)/$(STORE_FILES_TO)"

installer-prepare-lin32: installer-prepare
installer-prepare-win32: installer-prepare
installer-prepare-arm32: installer-prepare

installer-make-lin32:
	$(MAKE) -C ../ipnoise-packer clean
	$(MAKE) -C ../ipnoise-packer lin32
	$(MAKE) -C ../ipnoise-packer install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C ../ipnoise-xz 	 lin32
	$(MAKE) -C ../ipnoise-xz 	 install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C $(APPNAME) lin32

installer-make-win32:
	$(MAKE) -C ../ipnoise-packer win32
	$(MAKE) -C ../ipnoise-packer install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C $(APPNAME) win32

installer-make-arm32:
	$(MAKE) -C ../ipnoise-packer arm32
	$(MAKE) -C ../ipnoise-packer install DESTDIR="$(PWD)/$(STORE_FILES_TO)"
	$(MAKE) -C $(APPNAME) arm32

installer-install:
	install -m 0755 -d $(DESTDIR)

installer-install-lin32: installer-install
	install -m 0755 $(APPNAME)/stage1/stage1 $(DESTDIR)/stage1
	install -m 0755 -d $(DESTDIR)/pipe
	install -m 0755 -d $(DESTDIR)/pipe/resources
	install -m 0755 $(APPNAME)/stage2/stage2 	$(DESTDIR)/pipe/
	install -m 0755 $(APPNAME)/stage2/stage2.sh	$(DESTDIR)/pipe/
	cp -r $(APPNAME)/stage2/resources/* $(DESTDIR)/pipe/resources

installer-install-win32: installer-install
	install -m 0755 $(APPNAME)/stage1/stage1 $(DESTDIR)/stage1.exe
	install -m 0755 -d $(DESTDIR)/pipe
	install -m 0755 -d $(DESTDIR)/pipe/resources
	install -m 0755 $(APPNAME)/stage2/stage2 $(DESTDIR)/pipe/stage2.exe
	cp -r $(APPNAME)/stage2/resources/* $(DESTDIR)/pipe/resources
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/liblzma-5.dll 			$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/iconv.dll 				$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_sjlj-1.dll	$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll 		$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtGui4.dll 			$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtCore4.dll 			$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtNetwork4.dll 		$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtXml4.dll 			$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libpng15-15.dll		$(DESTDIR)/pipe/
	install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/zlib1.dll		        $(DESTDIR)/pipe/

installer-install-arm32: installer-install
	install -m 0755 $(APPNAME)/stage1/stage1 $(DESTDIR)/stage1
	install -m 0755 -d $(DESTDIR)/pipe
	install -m 0755 -d $(DESTDIR)/pipe/resources
	install -m 0755 $(APPNAME)/stage2/stage2 $(DESTDIR)/pipe/stage2
	cp -r $(APPNAME)/stage2/resources/* $(DESTDIR)/pipe/resources

installer-clean:
	rm -rf $(DESTDIR)/*
	if [ -r "$(APPNAME)/Makefile" ];then	\
		make -C $(APPNAME) clean;			\
	fi

installer-clean-lin32: installer-clean
installer-clean-win32: installer-clean
installer-clean-arm32: installer-clean

## not used
installer-platform-changed:
installer-rpms:

