APPNAME =	builder
APPDIR  =
ARCHES  =	lin32 win32 arm32

# sudo required:
# morik ALL=(ALL) NOPASSWD: /home/morik/git/IPNoise/packages/ipnoise-builder/bin/create_image.sh
# morik ALL=(ALL) NOPASSWD: /bin/mount	 	*
# morik ALL=(ALL) NOPASSWD: /bin/umount 	*
# morik ALL=(ALL) NOPASSWD: /sbin/losetup	*

ROOTDIR					= $(DESTDIR)/tmp/pipe/
IMAGE_FILE				= $(ROOTDIR)/ipnoise.img
IMAGE_MNT				= $(PWD)/mnt/
LOOP_DEVICE				= /dev/loop5
IPNOISE_BINARY_PACKAGES =	../ipnoise-transport/.rpms	\
							../ipnoise-libevent/.rpms 	\
							../ipnoise-iproute2/.rpms 	\
							../ipnoise-fake/.rpms 		\
							../ipnoise-tcpdump/.rpms	\
							../ipnoise-router/.rpms		\
							../ipnoise-release/.rpms	\
							../ipnoise-net-tools/.rpms	\
							../ipnoise-libfuse/.rpms	\
							../ipnoise-thttpd/.rpms		\
							../ipnoise-libnet/.rpms		\
							../ipnoise-libpcap/.rpms	\
							../ipnoise-openssh/.rpms	\
							../ipnoise-squid/.rpms

HASH_PATHS=	rootfs	\
			${IPNOISE_BINARY_PACKAGES}/ 		\
			../ipnoise-packer					\
			../ipnoise-pipe						\
			../ipnoise-starter					\
			../ipnoise-installer				\
			../ipnoise-kernel

.PHONY =	builder-prepare						\
			builder-prepare-lin32				\
			builder-prepare-win32				\
			builder-prepare-arm32				\
			builder-platform-changed			\
			builder-make						\
			builder-make-lin32					\
			builder-make-win32					\
			builder-make-arm32					\
			builder-install						\
			builder-install-lin32				\
			builder-install-win32				\
			builder-install-arm32				\
			builder-rpms						\
			builder-clean						\
			builder-clean-lin32					\
			builder-clean-win32					\
			builder-clean-arm32					\
			mount								\
			umount								\
			deps-lin32							\
			deps-win32							\
			deps-arm32							\
			copy-rpms

include ../Makefile.mk

builder-prepare:
	@install -m 0755 -d $(IMAGE_MNT)

builder-prepare-lin32: builder-prepare 
builder-prepare-win32: builder-prepare
builder-prepare-arm32: builder-prepare

copy-rpms:
	## make rpms
	$(MAKE) -C../ rpms
	## recreate temporary root tree
	@rm -rf root
	## prepare rpms
	mkdir -p root/var/cache/yum/i386/17/updates/packages
	@find ${IPNOISE_BINARY_PACKAGES}/  										\
		| sort -u															\
		| xargs -e ls -l 													\
		| openssl dgst -hex -md5 											\
		| awk '{print $$2}' > target_rpms.hash
	if 		! 	[ -r "target_rpms.hash" ] 									\
		|| 	! 	[ -r "last_rpms.hash"	]									\
		||		[ "`cat target_rpms.hash`" != "`cat last_rpms.hash`" ];		\
	then																	\
		rm -rf root/var/cache/yum/i386/17/updates/packages/*;				\
		for i in $(IPNOISE_BINARY_PACKAGES);do								\
			rpm=`find $$i -name "*.rpm" | grep -v debuginfo`;				\
			cp -f $$rpm root/var/cache/yum/i386/17/updates/packages/ 		\
				|| exit 1;													\
		done;																\
		cat target_rpms.hash > last_rpms.hash;								\
	fi

builder-make:
builder-make-lin32: builder-make
	$(MAKE) -C ../ipnoise-packer 	clean
	$(MAKE) -C ../ipnoise-installer lin32
	@## packer must be called after installer
	$(MAKE) -C ../ipnoise-packer 	clean
	$(MAKE) -C ../ipnoise-packer 	lin32 CFLAGS=-DCOMPRESS_MULTITHREADS
	$(MAKE) -C ../ipnoise-pipe 		lin32
	$(MAKE) -C ../ipnoise-qemu	 	lin32
	$(MAKE) -C ../ipnoise-starter 	lin32
	$(MAKE) -C ../ipnoise-kernel 	lin32
	## copy rpms
	$(MAKE) copy-rpms

builder-make-win32: builder-make
	$(MAKE) -C ../ipnoise-packer 	clean
	$(MAKE) -C ../ipnoise-installer win32
	@## packer must be called after installer
	$(MAKE) -C ../ipnoise-packer 	clean
	$(MAKE) -C ../ipnoise-packer 	lin32 CFLAGS=-DCOMPRESS_MULTITHREADS
	$(MAKE) -C ../ipnoise-pipe 		win32
	$(MAKE) -C ../ipnoise-qemu	 	win32
	$(MAKE) -C ../ipnoise-starter 	win32
	$(MAKE) -C ../ipnoise-kernel 	lin32
	$(MAKE) -C ../ipnoise-tvnc	 	win32
	## copy rpms
	$(MAKE) copy-rpms

builder-make-arm32: builder-make
	$(MAKE) -C ../ipnoise-packer 	clean
	$(MAKE) -C ../ipnoise-installer lin32
	@## packer must be called after installer
	$(MAKE) -C ../ipnoise-packer 	clean
	$(MAKE) -C ../ipnoise-packer 	lin32 CFLAGS=-DCOMPRESS_MULTITHREADS
	$(MAKE) -C ../ipnoise-pipe 		lin32
	$(MAKE) -C ../ipnoise-qemu	 	lin32
	$(MAKE) -C ../ipnoise-starter 	lin32
	$(MAKE) -C ../ipnoise-kernel 	lin32
	## copy rpms
	$(MAKE) copy-rpms

builder-install:
	@if [ ! -r "$(DESTDIR)" ]; then											\
			echo "Destination dir: '$(DESTDIR)' not exist";					\
   			echo "use DESTDIR=/path/to/destination/dir to specify";			\
		exit 1;																\
	fi
	rm -rf "$(ROOTDIR)"
	install -m 0755 -d "$(DESTDIR)"
	install -m 0755 -d "$(DESTDIR)/tmp"
	install -m 0755 -d "$(ROOTDIR)"
	## copy scripts
	@install -m 0644 rootfs/server.c		root/
	@install -m 0644 rootfs/test.c 			root/
	@install -m 0644 rootfs/test.pl 		root/
	sudo IMAGE_FILE=$(IMAGE_FILE) bin/create_image.sh

builder-install-lin32:	builder-install
	$(MAKE) -C ../ipnoise-packer 	install  	DESTDIR="$(DESTDIR)/tmp/"
	$(MAKE) -C ../ipnoise-installer install 	DESTDIR="$(DESTDIR)/tmp/"
	$(MAKE) -C ../ipnoise-pipe 		install		DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-qemu	 	install		DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-starter 	install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-kernel 	install 	DESTDIR="$(DESTDIR)/tmp/kernel"
	cp -f $(DESTDIR)/tmp/kernel/boot/vmlinuz $(ROOTDIR)/
	## call packer
	cd $(DESTDIR)/tmp											\
		&& ./packer             -a add -p stage1 -o ../setup	\
		&& ./packer -i ../setup -a add -p pipe -o ../setup;
	cd $(DESTDIR)   	\
		&& chmod +x setup;
	## remove tmp files
	rm -rf $(DESTDIR)/tmp

builder-install-win32:	builder-install
	$(MAKE) -C ../ipnoise-packer 	install		DESTDIR="$(DESTDIR)/tmp/"
	$(MAKE) -C ../ipnoise-installer install 	DESTDIR="$(DESTDIR)/tmp/"
	$(MAKE) -C ../ipnoise-pipe	 	install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-qemu 		install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-starter 	install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-tvnc	 	install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-kernel 	install 	DESTDIR="$(DESTDIR)/tmp/kernel"
	cp -f $(DESTDIR)/tmp/kernel/boot/vmlinuz $(ROOTDIR)/
	## call packer
	cd $(DESTDIR)/tmp															\
		&& ./packer                 -a add -p stage1.exe 	-o ../setup.exe		\
		&& ./packer -i ../setup.exe -a add -p pipe/pipe.exe -o ../setup.exe;
	cd $(DESTDIR)   \
		&& chmod +x setup.exe;
	## remove tmp files
	rm -rf $(DESTDIR)/tmp

builder-install-arm32:	builder-install
	$(MAKE) -C ../ipnoise-packer 	install  	DESTDIR="$(DESTDIR)/tmp/"
	$(MAKE) -C ../ipnoise-installer install		DESTDIR="$(DESTDIR)/tmp/"
	$(MAKE) -C ../ipnoise-pipe	 	install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-qemu	 	install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-starter 	install 	DESTDIR="$(ROOTDIR)"
	$(MAKE) -C ../ipnoise-kernel 	install 	DESTDIR="$(DESTDIR)/tmp/kernel"
	cp -f $(DESTDIR)/tmp/kernel/boot/vmlinuz $(ROOTDIR)/
	## call packer
	cd $(DESTDIR)/tmp											\
		&& ./packer 			-a add -p stage1 -o ../setup	\
		&& ./packer -i ../setup -a add -p pipe -o ../setup;
	cd $(DESTDIR)   	\
		&& chmod +x setup;
	## remove tmp files
	rm -rf $(DESTDIR)/tmp

mount: prepare umount $(DESTDIR)
	sudo /sbin/losetup -o 32256 $(LOOP_DEVICE) $(IMAGE_FILE)
	sudo /bin/mount $(LOOP_DEVICE) $(IMAGE_MNT)

umount: prepare $(DESTDIR)
	sudo /bin/umount 	-f $(LOOP_DEVICE) 	>/dev/null 2>&1; echo -n
	sudo /sbin/losetup 	-d $(LOOP_DEVICE)	>/dev/null 2>&1; echo -n

builder-clean:
	rm -f *.hash;

builder-clean-lin32:	builder-clean
builder-clean-win32:	builder-clean
builder-clean-arm32:	builder-clean

## not used
builder-rpms:
builder-platform-changed:

