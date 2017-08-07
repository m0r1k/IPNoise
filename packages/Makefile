TARGETS=	iproute2 			\
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
			openssh				\
			squid

ARCHES=lin32 win32 arm32
TARGETS_RPMS= $(TARGETS:=-rpms)

OUT=/mnt/multimedia/out/

.PHONY = usage rpms $(TARGETS) $(TARGETS_RPMS)	\
	lin32 	lin32_prepare 	lin32_make			\
	win32 	win32_prepare	win32_make			\
	arm32 	arm32_prepare	arm32_make

usage:
	@echo "Compilation: make $(ARCHES)"
	@echo "Clean:       make clean"

rpms: $(TARGETS_RPMS)

$(TARGETS_RPMS):
	$(MAKE) -C ipnoise-$(@:-rpms=) rpms

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

win32_prepare:
	install -m 0755 -d $(OUT)/
	install -m 0755 -d $(OUT)/win32
	install -m 0755 -d $(OUT)/win32/ipnoise

win32: win32_prepare rpms
	make -C ipnoise-builder win32
	make -C ipnoise-builder install DESTDIR=$(OUT)/win32/ipnoise/

clean:
	for i in `find . -maxdepth 1 -type d | grep "./"`;do	\
		make -C $$i clean;	\
	done

