%define package_name    linphone
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-linphone
%define version         3.5.2
%define release         0

Name:           %{name}
Summary:        IPNoise linphone implementation
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise linphone implementation
TODO add description here

%build
mkdir -p %buildroot
make -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
rm -rf %buildroot/usr/share/man
rm -rf %buildroot/usr/lib/debug
rm -rf %buildroot/usr/src/debug
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files -f %{package_path}/.files.list

%clean
rm -rf $RPM_BUILD_ROOT

