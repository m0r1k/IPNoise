#!/bin/sh

# sudo required:
# morik ALL=(ALL) NOPASSWD: bin/create_image.sh 

if [ "x" == "x$PWD" ];then
    PWD=`pwd`
fi

IMAGE_MNT=${PWD}/mnt/
IMAGE_CLEAR=${PWD}/.image_clear
IMAGE_RPMS=${PWD}/.image_rpms
IPNOISE_RPMS_DIR=${PWD}/root/var/cache/yum/i386/17/updates/packages/
if [ ! -n "${IMAGE_FILE}" ];then
    echo "unknown image file, use IMAGE_FILE=path_to_image_file";
    exit 1;
fi
LOOP_DEVICE=/dev/loop5
IMAGE_SIZE_MB=1024

RPM="rpm --root ${IMAGE_CLEAR}"
YUM="yum install -y --installroot=${IMAGE_CLEAR} --releasever=/"
YUM_DOWNLOAD="yum install ${YUM} --downloadonly --downloaddir=${IMAGE_RPMS}"

## -------------------------------- functions -------------------------------80

function pinfo
{
    echo "[ INFO  ] $@";
}

function pfatal
{
    echo "[ ERROR ] $@";
    exit 1;
}

function calculate_image_clear_hash
{
    IMAGE_CLEAR_HASH=`find ${IMAGE_CLEAR}/ root/ \
        | xargs -e ls -l \
        | openssl dgst -hex -md5 \
        | awk '{print $2}'`
}

## -------------------------------- main ------------------------------------80

if [ "x" == "x$PWD" ];then
    pfatal "cannot determine PWD path"
fi

if [ "0" != "`id -u`" ];then
    pfatal "root priveileges are required";
fi

if [ "ipnoise-builder" != "`basename $PWD`" ];then
    pfatal "start only from ipnoise-builder dir allowed";
fi

## check changes
calculate_image_clear_hash;
if      [ -r "${IMAGE_FILE}" ]  \
    &&  [ -r "${IMAGE_FILE}.${IMAGE_CLEAR_HASH}" ];
then
    pinfo "image not changed, skiped";
    exit 0;
fi

install -m 0755 -d ${IMAGE_CLEAR}
install -m 0755 -d ${IMAGE_CLEAR}/boot/
install -m 0755 -d ${IMAGE_CLEAR}/var/
install -m 0755 -d ${IMAGE_CLEAR}/var/lib/
install -m 0755 -d ${IMAGE_CLEAR}/var/cache/
#cp -r /var/cache/yum/ ${IMAGE_CLEAR}/var/cache/
${RPM} --initdb
#${RPM} --import public.gpg.key

${YUM} yum rpm filesystem systemd passwd pciutils crontabs.noarch \
    git qt less perl vim screen make gcc strace gdb \
    || exit 1

## remove conflict rpms
${RPM} -e --nodeps net-tools
${RPM} -e --nodeps iproute
${RPM} -e --nodeps hostname
${RPM} -e --nodeps openssh-clients
${RPM} -e --nodeps openssh

## install ipnoise-release
${RPM} -e ipnoise-release >/dev/null 2>&1
ls -l ${IPNOISE_RPMS_DIR}
${RPM} -h -i `find ${IPNOISE_RPMS_DIR}/ -name ipnoise-release-*.rpm | grep -v debuginfo` || exit 1

## remove old hash-files
rm -f ${IMAGE_FILE}.*

## create image
dd if=/dev/zero of=${IMAGE_FILE} bs=1M count=${IMAGE_SIZE_MB}
sync

## create partitions table
parted -a opt -s ${IMAGE_FILE} unit B mklabel msdos mkpart primary ext4 32256 100%
sync

## setup loop
/sbin/losetup -o 32256 ${LOOP_DEVICE} ${IMAGE_FILE}
IMAGE_PART_SIZE=`parted -s ${IMAGE_FILE} unit B print | grep primary | awk '{print $4}' | sed -e 's/B//g'`

## create fs
/sbin/mkfs.ext4 -F -b 1024 ${LOOP_DEVICE} # $((IMAGE_PART_SIZE/1024))
sync

## free loop
/sbin/losetup -d ${LOOP_DEVICE}

## setup image
chown morik:morik ${IMAGE_FILE}
make mount IMAGE_FILE="${IMAGE_FILE}"
rm -rf ${IMAGE_MNT}/*
cp -r ${IMAGE_CLEAR}/* ${IMAGE_MNT}/
cp -r root/* ${IMAGE_MNT}/
chroot ${IMAGE_MNT} bash -c "echo root | /bin/passwd --stdin root"

make umount IMAGE_FILE="${IMAGE_FILE}"

## create hash-file
calculate_image_clear_hash;
touch "${IMAGE_FILE}.${IMAGE_CLEAR_HASH}"

