%define package_name    libnet
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-libnet
%define version         1.1.2
%define release         3

Name:           %{name}
Summary:        IPNoise libnet
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise libnet
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

