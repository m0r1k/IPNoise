%define package_name    router
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-router
%define version         0.0.24
%define release         0

Name:           %{name}
Summary:        IPNoise router
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman E. Chechnev

%description
IPNoise router
TODO add description here

%build
mkdir -p %buildroot
make -j4 -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files
/etc/ipnoise/config.xml.example
/lib/systemd/system/ipnoise-router.service
/usr/sbin/ipnoise-router

%clean
rm -rf $RPM_BUILD_ROOT

%post
    mkdir -p /etc/ipnoise/profile/;                                         \
    if [ ! -r "/etc/ipnoise/profile/config.xml" ];then                      \
        cp /etc/ipnoise/config.xml.example /etc/ipnoise/profile/config.xml; \
    fi
    systemctl daemon-reload
    systemctl enable ipnoise-router
    systemctl restart ipnoise-router.service

%preun
    systemctl disable   ipnoise-router
    systemctl stop      ipnoise-router.service

%postun
    rm -f /etc/systemd/system/multi-user.target.wants/ipnoise-router.service
    systemctl daemon-reload

