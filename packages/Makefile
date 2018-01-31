export IPNOISE_PACKAGES_PATH=$(PWD)
export PATH := $(IPNOISE_PACKAGES_PATH)/bin:$(PATH)

TARGETS=	kernel				\
			iproute2 			\
			libevent 			\
			libfuse				\
		 	libnet				\
			libpcap				\
			net-tools			\
			tcpdump				\
			router				\
			transport			\
			release				\
			thttpd				\
			fake				\
			openssh
# TODO			squid

ARCHES=lin32 lin64 win32 arm32
TARGETS_RPMS=$(TARGETS:=-rpms)

OUT=./out/

.PHONY: setup usage rpms $(TARGETS) $(TARGETS_RPMS)	\
	qemu-lin32	qemu-lin64						\
	install-lin32 install-lin64					\
	lin32 	lin32_prepare 	lin32_make			\
	lin64 	lin64_prepare 	lin64_make			\
	win32 	win32_prepare	win32_make			\
	arm32 	arm32_prepare	arm32_make			\
	copy_rpms

lin32: IPNOISE_PACKAGES_ARCH="i386"
lin32: IPNOISE_PACKAGES_TARGET="lin32"
lin64: IPNOISE_PACKAGES_ARCH="x86_64"
lin64: IPNOISE_PACKAGES_TARGET="lin64"
win32: IPNOISE_PACKAGES_ARCH="i386"
win32: IPNOISE_PACKAGES_TARGET="win32"
arm32: IPNOISE_PACKAGES_ARCH="i386"
arm32: IPNOISE_PACKAGES_TARGET="arm32"
export IPNOISE_PACKAGES_ARCH
export IPNOISE_PACKAGES_TARGET

usage:
	@echo "Setup:       make setup"
	@echo "Compilation: make $(ARCHES)"
	@echo "Clean:       make clean"

rpms: $(TARGETS_RPMS)

setup:
	@echo "install required packages"
	sudo dnf install rpm-build      			\
		bison flex libdb-devel libdb4-devel     \
		gcc gcc-c++ make automake               \
		openssl openssl-devel					\
		libxml2 libxml2-devel       			\
		lzma									\
		bc										\
		qt5 qt5-devel qt5-qtbase-devel 			\
			qt5-qtbase-static 					\
			qt5-qtdeclarative-static			\
		libcap-devel libattr-devel				\
		libstdc++-static glibc-static			\
		syslinux-nonlinux						\
		libguestfs-tools
	@echo "prepare rpmbuild"
	mkdir -p ~/rpmbuild/BUILD/
	ln -sf $(IPNOISE_PACKAGES_PATH)/../../IPNoise /home/$(USER)/rpmbuild/BUILD/

$(TARGETS_RPMS):
	$(MAKE) -C ipnoise-$(@:-rpms=) rpms

## -------------- qemu --------------
qemu-lin32:
	$(MAKE) -C ipnoise-qemu lin32
	$(MAKE) -C ipnoise-qemu install

qemu-lin64:
	$(MAKE) -C ipnoise-qemu lin64
	$(MAKE) -C ipnoise-qemu install

## -------------- router --------------
router: libfuse libnet
	$(MAKE) -C ipnoise-$@

## -------------- transport --------------
transport: libevent
	$(MAKE) -C ipnoise-$@

## -------------- iproute2 --------------
iproute2:
	$(MAKE) -C ipnoise-$@

## -------------- libevent --------------
libevent:
	$(MAKE) -C ipnoise-$@

## -------------- libfuse --------------
libfuse:
	$(MAKE) -C ipnoise-$@

## -------------- libnet --------------
libnet:
	$(MAKE) -C ipnoise-$@

## -------------- net-tools --------------
net-tools:
	$(MAKE) -C ipnoise-$@

## -------------- libpcap --------------
libpcap:
	$(MAKE) -C ipnoise-$@

## -------------- tcpdump --------------
tcpdump: libpcap
	$(MAKE) -C ipnoise-$@

## -------------- release --------------
release:
	$(MAKE) -C ipnoise-$@

## -------------- thttpd --------------
thttpd:
	$(MAKE) -C ipnoise-$@

## -------------- fake --------------
fake:
	$(MAKE) -C ipnoise-$@

## -------------- openssh --------------
openssh:
	$(MAKE) -C ipnoise-$@

## -------------- ipnoise-squid --------------
squid:
	$(MAKE) -C ipnoise-$@

## -------------- ipnoise-linphone --------------
linphone:
	$(MAKE) -C ipnoise-$@

arm32_prepare:
	install -m 0755 -d $(OUT)/
	install -m 0755 -d $(OUT)/arm32
	install -m 0755 -d $(OUT)/arm32/ipnoise

arm32: arm32_prepare rpms
	make -C ipnoise-builder arm32
	make -C ipnoise-builder install DESTDIR=$(OUT)/arm32/ipnoise/

lin32_prepare:
	install -m 0755 -d $(OUT)/
	install -m 0755 -d $(OUT)/lin32
	install -m 0755 -d $(OUT)/lin32/ipnoise

lin32: lin32_prepare rpms
	make -C ipnoise-builder lin32
	make -C ipnoise-builder install DESTDIR=$(OUT)/lin32/ipnoise/

lin64_prepare:
	install -m 0755 -d $(OUT)/
	install -m 0755 -d $(OUT)/lin64
	install -m 0755 -d $(OUT)/lin64/ipnoise

lin64: lin64_prepare rpms
	make -C ipnoise-builder lin64
	make -C ipnoise-builder install DESTDIR=$(OUT)/lin64/ipnoise/

win32_prepare:
	install -m 0755 -d $(OUT)/
	install -m 0755 -d $(OUT)/win32
	install -m 0755 -d $(OUT)/win32/ipnoise

win32: win32_prepare rpms
	make -C ipnoise-builder win32
	make -C ipnoise-builder install DESTDIR=$(OUT)/win32/ipnoise/

install-lin32:
	mkdir -p ./out/lin32/ipnoise/pipe/
	make -C ipnoise-installer install DESTDIR=$(IPNOISE_PACKAGES_PATH)/out/lin32/ipnoise/
	make -C ipnoise-qemu	  lin32
	make -C ipnoise-qemu	  install DESTDIR=$(IPNOISE_PACKAGES_PATH)/out/lin32/ipnoise/pipe/
#	cp -f /usr/share/syslinux/ldlinux.c32 $(ipnoise_packages_path)/out/lin32/ipnoise/pipe/

install-lin64:
	mkdir -p ./out/lin64/ipnoise/pipe/
	make -C ipnoise-installer install DESTDIR=$(IPNOISE_PACKAGES_PATH)/out/lin64/ipnoise/
	make -C ipnoise-qemu	  lin64
	make -C ipnoise-qemu	  install DESTDIR=$(IPNOISE_PACKAGES_PATH)/out/lin64/ipnoise/pipe/

copy_rpms:
	make -C ipnoise-builder copy_rpms

clean:
	for i in `find . -maxdepth 1 -type d | grep "./"`;do	\
		make -C $$i clean;	\
	done


