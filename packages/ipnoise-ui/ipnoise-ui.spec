%define package_name    ui
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-ui
%define version         0.0.1
%define release         0

Name:           %{name}
Summary:        IPNoise UI
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman E. Chechnev

%description
IPNoise UI
TODO add description here

%build
mkdir -p %buildroot
make -j4 -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files
/usr/sbin/ipnoise-ui

%clean
rm -rf $RPM_BUILD_ROOT

%post

%preun

%postun

