APPNAME	= 	pipe
APPDIR 	= 	./pipe-rc

STORE_FILES_TO	= .files
ARCHES			= lin32 win32 arm32
HASH_PATHS		= ../ipnoise-common/ ${APPNAME}/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

DEFAULT_IMAGES=resources/default/images
DEFAULT_SOUNDS=resources/default/sounds

.PHONY =	default						\
			pipe-prepare-lin32			\
			pipe-prepare-win32			\
			pipe-prepare-arm32			\
			pipe-platform-changed		\
			pipe-make-lin32				\
			pipe-make-win32				\
			pipe-make-arm32				\
			pipe-install				\
			pipe-install-lin32			\
			pipe-install-win32			\
			pipe-install-arm32			\
			pipe-rpms					\
			pipe-clean

include ../Makefile.mk

pipe-make-lin32:
	$(MAKE) -C $(APPNAME) lin32

pipe-make-win32:
	$(MAKE) -C $(APPNAME) win32

pipe-make-arm32:
	$(MAKE) -C $(APPNAME) arm32

pipe-install:
	@install -m 0755 -d $(DESTDIR)/resources;
	@install -m 0755 -d $(DESTDIR)/resources/default;
	## images
	@install -m 0755 -d $(DESTDIR)/$(DEFAULT_IMAGES);
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_0.png					$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_135.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_180.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_225.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_270.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_315.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_45.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001_90.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_001.png					$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/loading_002.png					$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_addContact_intro.png		$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_addGroup_process.png		$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_add_intro.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_addLink_intro.png			$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_addLink_process.png		$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_addLink_process_tcp.png	$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_addLink_process_udp.png	$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_addLink_select.png			$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_add_process.png			$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_add_select.png				$(DESTDIR)/$(DEFAULT_IMAGES)/;
	@install -m 0644 $(APPNAME)/$(DEFAULT_IMAGES)/wizard_empty.png					$(DESTDIR)/$(DEFAULT_IMAGES)/;
	## sounds
	@install -m 0755 -d $(DESTDIR)/$(DEFAULT_SOUNDS);
	@install -m 0644 $(APPNAME)/$(DEFAULT_SOUNDS)/new_msg.wav						$(DESTDIR)/$(DEFAULT_SOUNDS)/;

pipe-install-win32: pipe-install
		install -m 0755 $(APPNAME)/pipe-rc $(DESTDIR)/pipe.exe;				\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_sjlj-1.dll	$(DESTDIR)/;	\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll 		$(DESTDIR)/;	\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtGui4.dll 			$(DESTDIR)/;	\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtCore4.dll 			$(DESTDIR)/;	\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtNetwork4.dll 		$(DESTDIR)/;	\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/QtXml4.dll 			$(DESTDIR)/;	\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/libpng15-15.dll		$(DESTDIR)/;	\
		install -m 0644 /usr/i686-w64-mingw32/sys-root/mingw/bin/zlib1.dll		        $(DESTDIR)/;

pipe-install-lin32: pipe-install
	install -m 0755 $(APPNAME)/pipe-rc  $(DESTDIR)/pipe;

pipe-install-arm32: pipe-install
	install -m 0755 $(APPNAME)/pipe-rc  $(DESTDIR)/pipe;

pipe-clean:
	rm -rf $(DESTDIR)/*
	make -C $(APPNAME) clean

pipe-clean-lin32: pipe-clean
pipe-clean-win32: pipe-clean
pipe-clean-arm32: pipe-clean

## not used
pipe-prepare-lin32:
pipe-prepare-win32:
pipe-prepare-arm32:
pipe-platform-changed:
pipe-rpms:

