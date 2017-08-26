%define package_name    tcpdump
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-tcpdump
%define version         2.0.6
%define release         1

Name:           %{name}
Summary:        IPNoise tcpdump
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise tcpdump
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

