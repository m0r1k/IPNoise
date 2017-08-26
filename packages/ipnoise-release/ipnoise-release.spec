%define package_name    release
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-release
%define version         0.0.12
%define release         16

Name:           %{name}
Summary:        IPNoise release
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise
Requires:       crontabs, passwd, xorg-x11-server-Xorg, xorg-x11-drv-vesa, mesa-dri-drivers

# TODO
# yum install firefox xorg-x11-font*

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise release
TODO add description here

%build
mkdir -p %buildroot
make -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files
/etc/init.d/ipnoise
/etc/ipnoise/profile/
/etc/inittab.ipnoise
/etc/cron.hourly/10ipnoise.sh
/usr/sbin/ipnoise-init
/usr/sbin/ipnoise-update
/usr/sbin/ipnoise-update-devel
/usr/sbin/ipnoise-first-init
/usr/sbin/ipnoise-setup-user
/usr/sbin/ipnoise-setup-item
/usr/sbin/ipnoise-setup-route
/usr/sbin/ipnoise-setup-neigh
/usr/share/vim/vimrc.ipnoise

%clean
rm -rf $RPM_BUILD_ROOT

%post
    if [ ! -r "/etc/ipnoise/.ipnoise" ]; then echo "root" | passwd --stdin root;fi

    grep "/usr/sbin/ipnoise-update" /etc/crontab >/dev/null 2>&1 \
        || echo "*/10 * * * * root /usr/sbin/ipnoise-update >/dev/null 2>&1" >> /etc/crontab

    grep "fuse" /etc/modprobe.preload >/dev/null 2>&1 \
        || echo "fuse" >> /etc/modprobe.preload

    grep "tun" /etc/modprobe.preload >/dev/null 2>&1 \
        || echo "tun" >> /etc/modprobe.preload

    grep "ipnoise_api" /etc/modprobe.preload >/dev/null 2>&1 \
        || echo "ipnoise_api" >> /etc/modprobe.preload

    grep        "2210:0000:0000:0000:0000:0000:0000:0001" /etc/hosts >/dev/null 2>&1 \
        || echo "2210:0000:0000:0000:0000:0000:0000:0001 updater" >> /etc/hosts

    echo "NETWORKING=yes" > /etc/sysconfig/network

    /sbin/chkconfig --level 235 ipnoise on >/dev/null 2>&1 || echo -n
    mv -f /usr/share/vim/vimrc.ipnoise  /usr/share/vim/vimrc
    mv -f /etc/inittab.ipnoise          /etc/inittab

%preun
    grep -v "/usr/sbin/ipnoise-update"  > /etc/crontab.1            && mv /etc/crontab.1 /etc/crontab
    grep -v "fuse"                      > /etc/modprobe.preload.1   && mv /etc/modprobe.preload.1   /etc/modprobe.preload
    grep -v "tun"                       > /etc/modprobe.preload.1   && mv /etc/modprobe.preload.1   /etc/modprobe.preload
    grep -v "ipnoise_api"               > /etc/modprobe.preload.1   && mv /etc/modprobe.preload.1   /etc/modprobe.preload
    /sbin/service ipnoise stop              >/dev/null 2>&1     || echo -n
    /sbin/chkconfig --level 235 ipnoise off >/dev/null 2>&1     || echo -n

