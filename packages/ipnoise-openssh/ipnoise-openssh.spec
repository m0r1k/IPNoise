%define package_name    openssh
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-openssh
%define version         6.0p1
%define release         4

Name:           %{name}
Summary:        IPNoise openssh
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman E. Chechnev

%description
IPNoise openssh
TODO add description here

%build
mkdir -p %buildroot
make -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files -f %{package_path}/.files.list

%clean
rm -rf $RPM_BUILD_ROOT

%post
    adduser sshd >/dev/null 2>&1
    mkdir -p /etc/ssh
    /usr/bin/ssh-keygen -A -q
    ln -sf /lib/systemd/system/sshd.service \
        /etc/systemd/system/multi-user.target.wants/sshd.service
    systemctl daemon-reload
    systemctl restart sshd.service

%preun
    systemctl stop sshd.service

