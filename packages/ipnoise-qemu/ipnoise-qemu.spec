%define package_name    qemu
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-qemu
%define version         1.7.0
%define release         1

Name:           %{name}
Summary:        IPNoise qemu
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise qemu
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

%changelog
* Mon Oct 20 2014 - morik@
- New version 1.7.0-1
- Fix memory leak bugs
- Simple debug of memory usage

