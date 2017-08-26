%define package_name    repository
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-repository
%define version         0.0.1
%define release         1

Name:           %{name}
Summary:        IPNoise repository for yum
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

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
/etc/yum.repos.d/ipnoise.repo

%clean
rm -rf $RPM_BUILD_ROOT

%post

%preun

