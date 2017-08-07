#!/bin/sh

## set -x

err=""

## check args
if [ ! -n "${IMAGE_DIR_CLEAR}" ];then
  err="1"
fi

if [ ! -n "${STORE_RPMS_TO}" ];then
  err="1"
fi

if [ ! -n "${SKIP_RPM}" ];then
  err="1"
fi

if [ ! -n "${IMAGE_DIR}" ];then
  err="1"
fi

if [ ! -n "${DISTRIB_STABLE}" ];then
  err="1"
fi

if [ ! -n "${OUT_DIR}" ];then
  err="1"
fi

if [ -n "$err" ];then
    echo "Error, some variables was not set:"
    echo "IMAGE_DIR_CLEAR=${IMAGE_DIR_CLEAR}"
    echo "STORE_RPMS_TO=${STORE_RPMS_TO}"
    echo "SKIP_RPM=${SKIP_RPM}"
    echo "IMAGE_DIR=${IMAGE_DIR}"
    echo "DISTRIB_STABLE=${DISTRIB_STABLE}"
    echo "OUT_DIR=${OUT_DIR}"
    exit 1
fi

## go go go
chmod 0777 ${STORE_RPMS_TO}
chown root:root ${STORE_RPMS_TO}

install -m 0755 -d ${IMAGE_DIR_CLEAR}
install -m 0755 -d ${IMAGE_DIR_CLEAR}/dev
install -m 0555 -d ${IMAGE_DIR_CLEAR}/proc
install -m 0750 -d ${IMAGE_DIR_CLEAR}/root
install -m 0755 -d ${IMAGE_DIR_CLEAR}/var/lock/subsys/
install -m 0777 -d ${IMAGE_DIR_CLEAR}/sys
install -m 0777 -d ${IMAGE_DIR_CLEAR}/tmp
## install system software
urpmi --root=${IMAGE_DIR_CLEAR} 	    \
	--auto							    \
	--skip 	"/${SKIP_RPM}/"			    \
	glibc libstdc++6 passwd rpm rpm-helper kbd cronie bash
urpmi --root=${IMAGE_ClEAR_DIR} 	    \
	--force 						    \
	--allow-nodeps 					    \
	--skip 	"/.*/"					    \
	udev
urpmi --root=${IMAGE_DIR_CLEAR} 	    \
	--force 						    \
	--allow-nodeps 					    \
	--skip 	"/${SKIP_RPM}/"			    \
	initscripts util-linux-ng
urpmi --root=${IMAGE_DIR_CLEAR} 	    \
	--auto							    \
	--skip 	"/${SKIP_RPM}/"			    \
	sysvinit
urpmi --root=${IMAGE_DIR_CLEAR} 	    \
	--auto							    \
	--skip 	"/${SKIP_RPM}/"			    \
	libxml2_2 urpmi wget libnl1
## devel software
urpmi --root=${IMAGE_DIR_CLEAR} 	    \
	--auto							    \
	nc6 strace gdb tree
urpmi --root=${IMAGE_DIR_CLEAR} 	    \
	--auto							    \
	vim-enhanced
## remove image dir
rm -rf ${IMAGE_DIR}
install -m 0755 -d ${IMAGE_DIR}
## copy clear files (third-party rpm's content tree) to image dir
cp -rp ${IMAGE_DIR_CLEAR}/* ${IMAGE_DIR}/
## install ipnoise software
rpm -U --root=${IMAGE_DIR} \
	`find ${DISTRIB_STABLE}/ -name "ipnoise-*.rpm"`
## remove garbage
chroot ${IMAGE_DIR} /sbin/chkconfig --level 12345 network off 		>/dev/null 2>&1 		|| echo -n
chroot ${IMAGE_DIR} /sbin/chkconfig --level 12345 network-up off	>/dev/null 2>&1			|| echo -n
chroot ${IMAGE_DIR} /sbin/chkconfig --level 12345 network-auth off  >/dev/null 2>&1			|| echo -n
chroot ${IMAGE_DIR} rpm -e dbus libdbus-1_3 perl-Net-DBus perl-Hal-Cdroms >/dev/null 2>&1	|| echo -n
## create devices
mknod ${IMAGE_DIR}/dev/console c 5 		1	>/dev/null 2>&1	|| echo -n
mknod ${IMAGE_DIR}/dev/full    c 1 		7	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/loop0   b 7 		0	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/mem     c 1 		1	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/null    c 1 		3	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/ptmx    c 5 		2	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/ram0    b 1 		0	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/random  c 1 		8	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/rtc0    c 253 	0	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/tty	   c 5      0	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/tty     c 4      0	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/tty     c 4      1	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/urandom c 1      9	>/dev/null 2>&1 || echo -n
mknod ${IMAGE_DIR}/dev/zero    c 1      5	>/dev/null 2>&1 || echo -n
## create image
mkdir -p ${OUT_DIR}
bin/create_image.sh ${IMAGE_DIR} ${OUT_DIR}
install -m 0755 bin/ipnoise.sh	 ${OUT_DIR}/ipnoise.sh
## public stable image
if [ `hostname` == "chechnev.ru" ];then 	\
        rm -rf /home/sites/ru.ipnoise/download;			\
        cp -r out/ipnoise /home/sites/ru.ipnoise/download;		\
fi

