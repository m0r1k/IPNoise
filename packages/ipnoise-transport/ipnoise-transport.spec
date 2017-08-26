%define package_name    transport
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-transport
%define version         0.0.9
%define release         0

Name:           %{name}
Summary:        IPNoise transport
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise transport
TODO add description here

%build
mkdir -p %buildroot
make -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files
/lib/systemd/system/ipnoise-transport-tcp-v4.service
/lib/systemd/system/ipnoise-transport-udp-v4.service
/usr/sbin/ipnoise-transport-tcp-v4
/usr/sbin/ipnoise-transport-udp-v4

%clean
rm -rf $RPM_BUILD_ROOT

%post
    systemctl daemon-reload
    systemctl enable  ipnoise-transport-tcp-v4
    systemctl enable  ipnoise-transport-udp-v4
    systemctl restart ipnoise-transport-tcp-v4.service
    systemctl restart ipnoise-transport-udp-v4.service

%preun
    systemctl disable   ipnoise-transport-tcp-v4
    systemctl disable   ipnoise-transport-udp-v4
    systemctl stop      ipnoise-transport-tcp-v4.service
    systemctl stop      ipnoise-transport-udp-v4.service

%postun
    rm -f /etc/systemd/system/multi-user.target.wants/ipnoise-transport-tcp.service
    rm -f /etc/systemd/system/multi-user.target.wants/ipnoise-transport-udp.service
    systemctl daemon-reload 

