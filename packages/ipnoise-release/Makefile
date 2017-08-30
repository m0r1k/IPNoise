APPNAME	=	release
APPDIR 	=
CFLAGS 	= 	-I$(PWD)/../

STORE_FILES_TO	= .files
STORE_RPMS_TO	= .rpms
ARCHES			= lin32
HASH_PATHS		= rootfs/

## DESTDIR must be only absolute path
DESTDIR ?= $(PWD)/$(STORE_FILES_TO)

.PHONY:		release-prepare-lin32		\
			release-make-lin32			\
			release-install-lin32		\
			release-clean-lin32			\
			release-prepare-lin64		\
			release-make-lin64			\
			release-install-lin64		\
			release-clean-lin64			\
			release-platform-changed	\
			release-rpms				\

include ../Makefile.mk

## lin32

release-install-lin32:
	install -m 0755 -d $(DESTDIR)/etc/
	install -m 0755 -d $(DESTDIR)/etc/init.d/
	install -m 0755 -d $(DESTDIR)/etc/ipnoise/
	install -m 0755 -d $(DESTDIR)/etc/ipnoise/profile/
	install -m 0755 -d $(DESTDIR)/etc/cron.hourly/
	install -m 0755 -d $(DESTDIR)/usr/sbin/
	install -m 0755 -d $(DESTDIR)/usr/share/
	install -m 0755 -d $(DESTDIR)/usr/share/vim/
	install -m 0755 rootfs/etc/init.d/ipnoise				$(DESTDIR)/etc/init.d/ipnoise
	install -m 0644 rootfs/etc/inittab.ipnoise				$(DESTDIR)/etc/inittab.ipnoise
	install -m 0755 rootfs/etc/cron.hourly/10ipnoise.sh		$(DESTDIR)/etc/cron.hourly/10ipnoise.sh
	install -m 0755 rootfs/usr/sbin/ipnoise-init			$(DESTDIR)/usr/sbin/ipnoise-init
	install -m 0755 rootfs/usr/sbin/ipnoise-update			$(DESTDIR)/usr/sbin/ipnoise-update
	install -m 0755 rootfs/usr/sbin/ipnoise-update-devel	$(DESTDIR)/usr/sbin/ipnoise-update-devel
	install -m 0755 rootfs/usr/sbin/ipnoise-first-init		$(DESTDIR)/usr/sbin/ipnoise-first-init
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-user		$(DESTDIR)/usr/sbin/ipnoise-setup-user
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-item		$(DESTDIR)/usr/sbin/ipnoise-setup-item
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-route		$(DESTDIR)/usr/sbin/ipnoise-setup-route
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-neigh		$(DESTDIR)/usr/sbin/ipnoise-setup-neigh
	install -m 0644 rootfs/usr/share/vim/vimrc.ipnoise		$(DESTDIR)/usr/share/vim/vimrc.ipnoise

release-clean-lin32:
	rm -rf $(DESTDIR)/*

## lin64

release-install-lin64:
	install -m 0755 -d $(DESTDIR)/etc/
	install -m 0755 -d $(DESTDIR)/etc/init.d/
	install -m 0755 -d $(DESTDIR)/etc/ipnoise/
	install -m 0755 -d $(DESTDIR)/etc/ipnoise/profile/
	install -m 0755 -d $(DESTDIR)/etc/cron.hourly/
	install -m 0755 -d $(DESTDIR)/usr/sbin/
	install -m 0755 -d $(DESTDIR)/usr/share/
	install -m 0755 -d $(DESTDIR)/usr/share/vim/
	install -m 0755 rootfs/etc/init.d/ipnoise				$(DESTDIR)/etc/init.d/ipnoise
	install -m 0644 rootfs/etc/inittab.ipnoise				$(DESTDIR)/etc/inittab.ipnoise
	install -m 0755 rootfs/etc/cron.hourly/10ipnoise.sh		$(DESTDIR)/etc/cron.hourly/10ipnoise.sh
	install -m 0755 rootfs/usr/sbin/ipnoise-init			$(DESTDIR)/usr/sbin/ipnoise-init
	install -m 0755 rootfs/usr/sbin/ipnoise-update			$(DESTDIR)/usr/sbin/ipnoise-update
	install -m 0755 rootfs/usr/sbin/ipnoise-update-devel	$(DESTDIR)/usr/sbin/ipnoise-update-devel
	install -m 0755 rootfs/usr/sbin/ipnoise-first-init		$(DESTDIR)/usr/sbin/ipnoise-first-init
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-user		$(DESTDIR)/usr/sbin/ipnoise-setup-user
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-item		$(DESTDIR)/usr/sbin/ipnoise-setup-item
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-route		$(DESTDIR)/usr/sbin/ipnoise-setup-route
	install -m 0755 rootfs/usr/sbin/ipnoise-setup-neigh		$(DESTDIR)/usr/sbin/ipnoise-setup-neigh
	install -m 0644 rootfs/usr/share/vim/vimrc.ipnoise		$(DESTDIR)/usr/share/vim/vimrc.ipnoise

release-clean-lin64:
	rm -rf $(DESTDIR)/*

## not used
release-prepare-lin32:
release-make-lin32:
release-prepare-lin64:
release-make-lin64:
release-rpms:
release-platform-changed:

