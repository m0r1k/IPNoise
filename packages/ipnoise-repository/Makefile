APPNAME	=	repository
APPDIR 	=
CFLAGS 	= 	-I$(PWD)/../

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32 lin64
HASH_PATHS		= rootfs/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		repository-prepare-lin32		\
			repository-make-lin32			\
			repository-install-lin32		\
			repository-clean-lin32			\
			repository-prepare-lin64		\
			repository-make-lin64			\
			repository-install-lin64		\
			repository-clean-lin64			\
			repository-platform-changed		\
			repository-rpms

include ../Makefile.mk

## lin32

repository-install-lin32:
	install -m 0755 -d $(DESTDIR)/etc/
	install -m 0755 -d $(DESTDIR)/etc/yum.repos.d/
	install -m 0755 rootfs/etc/yum.repos.d/ipnoise.repo				$(DESTDIR)/etc/yum.repos.d/ipnoise.repo

repository-clean-lin32:
	rm -rf $(DESTDIR)/*

## lin64

repository-install-lin64:
	install -m 0755 -d $(DESTDIR)/etc/
	install -m 0755 -d $(DESTDIR)/etc/yum.repos.d/
	install -m 0755 rootfs/etc/yum.repos.d/ipnoise.repo				$(DESTDIR)/etc/yum.repos.d/ipnoise.repo

repository-clean-lin64:
	rm -rf $(DESTDIR)/*

## not used
repository-prepare-lin32:
repository-make-lin32:
repository-prepare-lin64:
repository-make-lin64:
repository-platform-changed:
repository-rpms:

