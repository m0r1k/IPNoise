APPNAME	=	repository
APPDIR 	=
CFLAGS 	= 	-I$(PWD)/../

RPMS_DIR		= ~/rpmbuild/RPMS/i386/
SRPMS_DIR		= ~/rpmbuild/SRPMS/
STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= rootfs/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY =	repository-prepare-lin32		\
			repository-platform-changed	\
			repository-make-lin32			\
			repository-install-lin32		\
			repository-rpms				\
			repository-clean-lin32

include ../Makefile.mk

repository-install-lin32:
	install -m 0755 -d $(DESTDIR)/etc/
	install -m 0755 -d $(DESTDIR)/etc/yum.repos.d/
	install -m 0755 rootfs/etc/yum.repos.d/ipnoise.repo				$(DESTDIR)/etc/yum.repos.d/ipnoise.repo

repository-clean-lin32:
	rm -rf $(DESTDIR)/*

## not used
repository-prepare-lin32:
repository-platform-changed:
repository-make-lin32:
repository-rpms:

