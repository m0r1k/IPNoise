APPNAME	= 	tvnc
APPDIR 	=

STORE_FILES_TO	= .files
ARCHES			= win32
HASH_PATHS		= ../ipnoise-common/ win32/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		tvnc-prepare-win32		\
			tvnc-platform-changed	\
			tvnc-make-win32			\
			tvnc-install-win32		\
			tvnc-rpms				\
			tvnc-clean-win32		\

include ../Makefile.mk

tvnc-install-win32:
	mkdir -p $(STORE_FILES_TO)
	install -m 0755 win32/hookldr.exe 			$(DESTDIR)/
	install -m 0644 win32/LICENSE.txt			$(DESTDIR)/
	install -m 0644 win32/screenhooks32.dll		$(DESTDIR)/
	install -m 0644 win32/screenhooks64.dll		$(DESTDIR)/
	install -m 0644 win32/tvnviewer.exe			$(DESTDIR)/

tvnc-clean-win32:
	rm -rf $(DESTDIR)/*


## not used
tvnc-prepare-win32:
tvnc-platform-changed:
tvnc-make-win32:
tvnc-rpms:

